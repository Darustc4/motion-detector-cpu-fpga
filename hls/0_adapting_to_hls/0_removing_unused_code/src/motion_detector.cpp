#include "motion_detector.hpp"

#include <iostream>

#include <chrono>
#include <stdexcept>
#include <cmath>

#include "image_utils.hpp"
#include "contour_detector.hpp"

namespace motdet
{
    // Motion_detector implementation

    Motion_detector::Motion_detector(const std::size_t width, const std::size_t height, const std::size_t threads, const std::size_t queue_size, const unsigned int downsample_factor, const float frame_update_ratio):
        w_(width),
        h_(height),
        total_(width*height),
        threads_(threads),
        queue_size_(queue_size),
        downsample_factor_(downsample_factor),
        frame_update_ratio_(frame_update_ratio),
        min_cont_area_(total_*0.002+5),
        last_submitted_time_(0)
    {
        if (threads    == 0)        throw std::invalid_argument("ERROR Constructor: threads must be at least 1.");
        if (queue_size == 0)        throw std::invalid_argument("ERROR Constructor: queue_size must be at least 1.");
        if (downsample_factor == 0) throw std::invalid_argument("ERROR Constructor: downsample_factor must be at least 1.");
        if (width      < 10)        throw std::invalid_argument("ERROR Constructor: width must be at least 10.");
        if (height     < 10)        throw std::invalid_argument("ERROR Constructor: height must be at least 10.");

        // The size requirements for the downsampled image are given by the function in image_utils.
        downsampled_w_ = std::ceil((float)w_ / downsample_factor);
        downsampled_h_ = std::ceil((float)h_ / downsample_factor);

        reference_ = Image<unsigned short>(downsampled_w_, downsampled_h_, {});

        // Create all the motion detector slaves.
        for(std::size_t i = 0; i < threads; ++i) workers_container_.push_back(std::thread(&Motion_detector::detect_motion_, this, i));
    }

    Motion_detector::~Motion_detector()
    {
        // The destructor will wait for all threads to die before destroying itself. Leaving a thread unhandled causes error unless it is a daemon.
        keep_workers_alive_ = false;
        no_processable_frame_.notify_all();
        for(std::thread &t : workers_container_) t.join();
    }

    void Motion_detector::enqueue_frame(std::unique_ptr<Image<unsigned short>> in, unsigned long long timestamp_millis, bool blocking, std::shared_ptr<void> data_keep)
    {
        if(in.get() == NULL) throw std::invalid_argument("ERROR Enqueue: The input image is NULL.");
        if(in->get_total() != total_ || in->get_width() != w_) throw std::invalid_argument("ERROR Enqueue: Wrong resolution.");

        // Lock mutex for checks
        std::unique_lock<std::mutex> locker(tasks_mutex_);
        if(timestamp_millis < last_submitted_time_) throw std::invalid_argument("ERROR Enqueue: Submitted timestamps must be chronologically ordered.");
        last_submitted_time_ = timestamp_millis;

        if(blocking)
        {
            // Blocking mode, sleep until queue is not full.
            tasks_full_cond_.wait(locker, [this](){ return task_queue_.size() < queue_size_; });
        }
        else
        {
            // Non-blocking, check if the queue is full and if it is, throw exception
            if(task_queue_.size() < queue_size_) throw std::runtime_error("ERROR Enqueue: Queue is full.");
        }
        // If reached this point, there is a spot available in the queue and the input is valid

        Motdet_task_ new_task;
        new_task.timestamp = timestamp_millis;
        new_task.image = std::move(in); // Need to move smart pointer with move to represent ownership transfer
        new_task.data_keep = data_keep; // Store the extra metadata but nothing will be done with it.

        task_queue_.push_back(std::move(new_task));
        locker.unlock();  // Release the lock and notfy one of the worker threads that there is a new available job to process.
        no_processable_frame_.notify_one();
    }

    Motion_detector::Detection Motion_detector::get_detection(bool blocking)
    {
        // Lock mutex for checks
        std::unique_lock<std::mutex> locker(results_mutex_);

        if(blocking)
        {
            // Blocking mode, sleep until the oldest frame is ready for output.
            results_empty_cond_.wait(locker, [this](){ return result_queue_.size() > 0; });
        }
        else
        {
            // Non-blocking, throw exception if the queue is empty or the oldest frame is not ready
            if(result_queue_.size() == 0) throw std::runtime_error("ERROR Enqueue: No results ready.");
        }
        // If reached here, it means there is a valid result to return

        auto result = result_queue_.front();
        result_queue_.pop_front();

        return result;
    }

    void Motion_detector::detect_motion_(std::size_t thread_id)
    {
        while(keep_workers_alive_)
        {
            // Grab a new task to process. It will need to grab the mutex to do so.

            std::unique_lock<std::mutex> tasks_locker(tasks_mutex_);
            no_processable_frame_.wait(tasks_locker, [this](){ return processable_frame_check_() || !keep_workers_alive_; });
            if(!keep_workers_alive_) break;

            // If the thread reached this point, there is at least 1 task that can be processed in the queue and it got "permission" to process it.

            std::deque<Motdet_task_>::iterator to_process = task_queue_.begin();
            while (to_process != task_queue_.end() && to_process->state != Motdet_task_::task_state::waiting) to_process++;
            if(to_process == task_queue_.end()) throw std::runtime_error("ERROR detect_motion_: No processable frame found but expected one.");

            // Successfully got the frame, now mark it so that other threads do not start processing it as well.
            to_process->state = Motdet_task_::task_state::processing;
            tasks_locker.unlock();

            // It is assured by program logic that this frame will not be edited by another thread now. Begin processing.
            auto processing_time_start = std::chrono::high_resolution_clock::now();

            // Get the input image and downsample it, if needed.
            Image<unsigned short> in(std::move(*to_process->image.get()));
            Image<unsigned short> downsampled_in(downsampled_w_, downsampled_h_, 0);

            if(downsample_factor_ > 1) imgutil::downsample(in, downsampled_in, downsample_factor_);
            else downsampled_in = std::move(in);

            // If the motion detector has a reference frame, compare with it to check for motion. If not, make a new reference.
            std::unique_lock<std::mutex> reference_locker(reference_mutex_);
            if(has_reference_)
            {
                reference_locker.unlock(); // Release the reference mutex since we will not edit the reference for a while.

                // Define all the container images for intermediate processing steps.
                Image<unsigned short> blur_image(downsampled_w_, downsampled_h_, 0), sub_image(downsampled_w_, downsampled_h_, 0), new_ref_image(downsampled_w_, downsampled_h_, 0);
                Image<unsigned char> thr_image(downsampled_w_, downsampled_h_, 0), cnt_image(downsampled_w_, downsampled_h_, 0);
                Image<int> dil_image(downsampled_w_, downsampled_h_, 0);

                // Blur the image to remove any noise that can result in false positives.
                if(!keep_workers_alive_) break;
                imgutil::gaussian_blur_filter<unsigned short, unsigned short>(downsampled_in, blur_image);

                // Subtract the blurred frame with the reference image. Leaving only the changes between frames.
                if(!keep_workers_alive_) break;
                reference_locker.lock();
                imgutil::image_subtraction<unsigned short, unsigned short>(blur_image, reference_, sub_image);
                reference_locker.unlock();

                // Interpolate the blurred image and the reference frame to obtain a new reference.
                // Interpolation is done so that the reference can adapt to changing environment.
                if(!keep_workers_alive_) break;
                reference_locker.lock();
                imgutil::image_interpolation<unsigned short, unsigned short>(reference_, blur_image, new_ref_image, frame_update_ratio_);
                reference_ = std::move(new_ref_image);
                reference_locker.unlock();

                // Threshold the image so that any value below a certain number is ignored.
                // Using double threshold along with hysteresis for better results over single threshold.
                if(!keep_workers_alive_) break;
                imgutil::double_threshold<unsigned short, unsigned char>(sub_image, thr_image, 8000, 25000);
                imgutil::hysteresis<unsigned char, unsigned char>(thr_image, cnt_image);

                // Dilate the image so that the contours are better defined and with less holes.
                if(!keep_workers_alive_) break;
                imgutil::dilation<unsigned char, int>(cnt_image, dil_image);

                // Detect contours in the image. Any contour detected here is "movement".
                if(!keep_workers_alive_) break;
                std::vector<Contour> unfiltered_contours;;
                imgutil::contour_detection(dil_image, unfiltered_contours);

                // Go over the detected contorus and discard any contour that is too small to be relevant.
                // Also scale the bounding box of the contour back to the original size before downscaling.
                if(!keep_workers_alive_) break;
                for(Contour &cont : unfiltered_contours)
                {
                    unsigned int cont_area = (cont.bb_tl_x - cont.bb_br_x) * (cont.bb_tl_y - cont.bb_br_y) * downsample_factor_;
                    if(cont_area > min_cont_area_)
                    {
                        cont.bb_tl_x *= downsample_factor_;
                        cont.bb_br_x *= downsample_factor_;
                        cont.bb_tl_y *= downsample_factor_;
                        cont.bb_br_y *= downsample_factor_;
                        to_process->result_conts.push_back(std::move(cont));
                    }
                }
            }
            else
            {
                // There is no reference, create a new one with the current input frame.

                if(!keep_workers_alive_) break;
                imgutil::gaussian_blur_filter<unsigned short, unsigned short>(downsampled_in, reference_);

                has_reference_ = true;
                reference_locker.unlock();
            }
            // Processing has ended here, the only thing missing is submitting the result.
            to_process->state = Motdet_task_::task_state::done;

            // Record the time it took the frame to be processed.
            if(!keep_workers_alive_) break;
            auto processing_time_end = std::chrono::high_resolution_clock::now();
            to_process->processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(processing_time_end - processing_time_start).count();

            // Now that this frame is finished, check from oldest to newest the state of the different tasks.
            // Submit the tasks to the result queue until a task with a state different from finished is found.
            // This assures that the results are outputted in chronological order, not processing order.
            // Note it might cause a thread to not submit any results, since the frame it jsut processed it too new.

            // Lock both the tasks queue and the results queue.
            tasks_locker.lock();
            std::unique_lock<std::mutex> results_locker(results_mutex_);

            std::deque<Motdet_task_>::iterator to_submit = task_queue_.begin();
            while (to_submit != task_queue_.end() && to_submit->state == Motdet_task_::task_state::done)
            {
                // For each finished task, create a new Detection struct and submit it to the results.
                Detection det;
                det.timestamp = to_submit->timestamp;
                det.processing_time = to_submit->processing_time;
                det.detection_contours = std::move(to_submit->result_conts);
                det.has_detections = det.detection_contours.size() > 0;
                det.data_keep = to_submit->data_keep;

                result_queue_.push_back(det);
                to_submit = task_queue_.erase(to_submit); // This updates the iterator to the next element automatically.
            }

            results_locker.unlock();
            tasks_locker.unlock();
            tasks_full_cond_.notify_all(); // Notifying all because we might have submitted more than 1 frame.
            results_empty_cond_.notify_all();
        }
    }


    // Other functions implementation

    void rgb_to_bw(const Image<rgb_pixel> &in, Image<unsigned short> &out)
    {
        for(std::size_t i = 0; i < in.get_total(); ++i)
        {
            rgb_pixel pixel = in[i];
            out[i] = pixel[0] * 76.245 + pixel[1] * 149.685 + pixel[2] * 29.07;
        }
    }


    void uchar_to_bw(const unsigned char *in, const std::size_t n_pix, Image<unsigned short> &out)
    {
        for(std::size_t i = 0; i < n_pix; ++i)
        {
            std::size_t mapped = i*3;
            out[i] = in[mapped] * 76.245 + in[mapped+1] * 149.685 + in[mapped+2] * 29.07;
        }
    }

} // namespace motdet
