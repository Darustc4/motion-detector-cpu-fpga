#include "motion_detector.hpp"

#include <iostream>
#include <iomanip>

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

        downsampled_w_ = std::ceil((float)w_ / downsample_factor);
        downsampled_h_ = std::ceil((float)h_ / downsample_factor);

        std::cout << "Using resolution " << downsampled_w_ << "x" << downsampled_h_ << std::endl;

        reference_ = Image<unsigned short>(downsampled_w_, downsampled_h_, {});

        // Create all the motion detector slaves.
        for(std::size_t i = 0; i < threads; ++i) workers_container_.push_back(std::thread(&Motion_detector::detect_motion_, this, i));
    }

    Motion_detector::~Motion_detector()
    {
        keep_workers_alive_ = false;
        no_processable_frame_.notify_all();
        for(std::thread &t : workers_container_) t.join();
    }

    void Motion_detector::enqueue_frame(std::unique_ptr<Image<unsigned short>> in, unsigned long long timestamp_millis, bool blocking)
    {
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

        motdet_task_ new_task;
        new_task.timestamp = timestamp_millis;
        new_task.image = std::move(in); // Need to move smart pointer with move to represent ownership transfer

        task_queue_.push_back(std::move(new_task));
        locker.unlock();  // Release the lock and notfy one of the worker threads that there is a new available job to process.
        no_processable_frame_.notify_one();
    }

    std::pair<unsigned long long, std::vector<Contour>> Motion_detector::get_detected_contours(bool blocking)
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
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;

        while(keep_workers_alive_)
        {
            std::unique_lock<std::mutex> tasks_locker(tasks_mutex_);
            no_processable_frame_.wait(tasks_locker, [this](){ return processable_frame_check_() || !keep_workers_alive_; });
            if(!keep_workers_alive_) break;

            // If here, there is at least 1 task that can be processed in the queue.

            std::deque<motdet_task_>::iterator to_process = task_queue_.begin();
            while (to_process != task_queue_.end() && to_process->state != motdet_task_::task_state::waiting) to_process++;
            if(to_process == task_queue_.end()) throw std::runtime_error("ERROR detect_motion_: No processable frame found but expected one.");

            to_process->state = motdet_task_::task_state::processing;
            tasks_locker.unlock();
            // Got a frame to process. It is assured by program logic that this frame won't be edited by another thread.

            Image<unsigned short> in(std::move(*to_process->image.get()));
            Image<unsigned short> downsampled_in(downsampled_w_, downsampled_h_, 0);

            auto t_frame0 = high_resolution_clock::now();
            auto t_res0 = high_resolution_clock::now();
            if(downsample_factor_ > 1) imgutil::downsample(in, downsampled_in, downsample_factor_);
            else downsampled_in = std::move(in);
            auto t_res1 = high_resolution_clock::now();

            // If the motion detector has a reference frame, use it to check of motion. If not, make a new reference.
            std::unique_lock<std::mutex> reference_locker(reference_mutex_);
            if(has_reference_)
            {
                reference_locker.unlock();

                Image<unsigned short> blur_image(downsampled_w_, downsampled_h_, 0), sub_image(downsampled_w_, downsampled_h_, 0), new_ref_image(downsampled_w_, downsampled_h_, 0);
                Image<unsigned char> thr_image(downsampled_w_, downsampled_h_, 0), cnt_image(downsampled_w_, downsampled_h_, 0), debug_image(downsampled_w_, downsampled_h_, 0);
                Image<int> dil_image(downsampled_w_, downsampled_h_, 0);

                // Blur the image to remove any noise that can result in false positives.
                auto t_gauss0 = high_resolution_clock::now();
                imgutil::gaussian_blur_filter<unsigned short, unsigned short>(downsampled_in, blur_image);
                if(!keep_workers_alive_) break;
                if(save_debug_images_)
                {
                    std::string debug_img_path = debug_dir_ / ("blurred_" + std::to_string(to_process->timestamp) + ".pgm");
                    imgutil::reescale_pix_length<unsigned short, unsigned char>(blur_image, debug_image, 65535, 255);
                    imgutil::save_image_8b(debug_img_path, debug_image);
                }
                auto t_gauss1 = high_resolution_clock::now();
                // Subtract the blurred frame with the reference image. Leaving only the changes in the image.
                auto t_sub0 = high_resolution_clock::now();
                reference_locker.lock();
                imgutil::image_subtraction<unsigned short, unsigned short>(blur_image, reference_, sub_image);
                if(!keep_workers_alive_) break;
                reference_locker.unlock();

                if(save_debug_images_)
                {
                    std::string debug_img_path = debug_dir_ / ("subbed_" + std::to_string(to_process->timestamp) + ".pgm");
                    imgutil::reescale_pix_length<unsigned short, unsigned char>(sub_image, debug_image, 65535, 255);
                    imgutil::save_image_8b(debug_img_path, debug_image);
                }
                auto t_sub1 = high_resolution_clock::now();
                // Interpolate the blurred image and the reference frame to obtain a new reference.
                // Only interpolate if the timestamp of the current frame is not older than the last time the reference was updated.
                auto t_int0 = high_resolution_clock::now();

                reference_locker.lock();
                long long time_diff = to_process->timestamp - last_ref_update_time_;
                imgutil::image_interpolation<unsigned short, unsigned short>(reference_, blur_image, new_ref_image, frame_update_ratio_);
                reference_ = std::move(new_ref_image);
                if(!keep_workers_alive_) break;

                if(save_debug_images_)
                {
                    std::string debug_img_path = debug_dir_ / ("reference_" + std::to_string(to_process->timestamp) + ".pgm");
                    imgutil::reescale_pix_length<unsigned short, unsigned char>(reference_, debug_image, 65535, 255);
                    imgutil::save_image_8b(debug_img_path, debug_image);
                }
                reference_locker.unlock();

                auto t_int1 = high_resolution_clock::now();
                // Threshold the image so that any value below a certain number is ignored.
                // Using double threshold with hysteresis for better results.
                auto t_thr0 = high_resolution_clock::now();
                imgutil::double_threshold<unsigned short, unsigned char>(sub_image, thr_image, 8000, 25000);
                if(!keep_workers_alive_) break;
                auto t_thr1 = high_resolution_clock::now();
                auto t_hys0 = high_resolution_clock::now();
                imgutil::hysteresis<unsigned char, unsigned char>(thr_image, cnt_image);
                if(!keep_workers_alive_) break;
                auto t_hys1 = high_resolution_clock::now();

                if(save_debug_images_)
                {
                    std::string debug_img_path = debug_dir_ / ("thresh_" + std::to_string(to_process->timestamp) + ".pgm");
                    imgutil::reescale_pix_length<unsigned char, unsigned char>(cnt_image, debug_image, 1, 255);
                    imgutil::save_image_8b(debug_img_path, debug_image);
                }

                auto t_dil0 = high_resolution_clock::now();
                imgutil::dilation<unsigned char, int>(cnt_image, dil_image);
                auto t_dil1 = high_resolution_clock::now();

                if(save_debug_images_)
                {
                    std::string debug_img_path = debug_dir_ / ("dilated_" + std::to_string(to_process->timestamp) + ".pgm");
                    imgutil::reescale_pix_length<int, unsigned char>(dil_image, debug_image, 1, 255);
                    imgutil::save_image_8b(debug_img_path, debug_image);
                }

                std::vector<Contour> unfiltered_contours;
                auto t_con0 = high_resolution_clock::now();
                imgutil::contour_detection(dil_image, unfiltered_contours);
                if(!keep_workers_alive_) break;
                auto t_con1 = high_resolution_clock::now();

                for(Contour &cont : unfiltered_contours)
                {
                    unsigned int cont_area = (cont.bb_tl_x - cont.bb_br_x) * (cont.bb_tl_y - cont.bb_br_y) * downsample_factor_;
                    if(cont_area > min_cont_area_) to_process->result_conts.push_back(std::move(cont));
                }

                for(Contour &cont : to_process->result_conts)
                {
                    cont.bb_tl_x *= downsample_factor_;
                    cont.bb_br_x *= downsample_factor_;
                    cont.bb_tl_y *= downsample_factor_;
                    cont.bb_br_y *= downsample_factor_;
                }

                auto t_frame1 = high_resolution_clock::now();

                std::cout << "Executed downscaling in " << duration_cast<milliseconds>(t_res1 - t_res0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed gaussian blur in " << duration_cast<milliseconds>(t_gauss1 - t_gauss0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed img subtract in  " << duration_cast<milliseconds>(t_sub1 - t_sub0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed interpolation in " << duration_cast<milliseconds>(t_int1 - t_int0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed double thresh in " << duration_cast<milliseconds>(t_thr1 - t_thr0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed hysteresis in    " << duration_cast<milliseconds>(t_hys1 - t_hys0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed dilation in      " << duration_cast<milliseconds>(t_dil1 - t_dil0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed contour detec in " << duration_cast<milliseconds>(t_con1 - t_con0).count() << " millis " << thread_id << std::endl;
                std::cout << "Executed ALL in           " << duration_cast<milliseconds>(t_frame1 - t_frame0).count() << " millis " << thread_id << std::endl;
                std::cout << "Finished frame " << to_process->timestamp << std::endl;
            }
            else
            {
                // Save the reference already blurred, this removes the noise from the image.

                imgutil::gaussian_blur_filter<unsigned short, unsigned short>(downsampled_in, reference_);
                if(!keep_workers_alive_) break;
                has_reference_ = true;
                reference_locker.unlock();
            }

            if(!keep_workers_alive_) break;
            to_process->state = motdet_task_::task_state::done;
            tasks_locker.lock();
            std::unique_lock<std::mutex> results_locker(results_mutex_);

            // Now that this frame is finished, check from oldest to newest the state of the tasks.
            // Submit the tasks to the result queue until a task with a state different from finished is found. Might submit nothing.
            std::deque<motdet_task_>::iterator to_submit = task_queue_.begin();
            while (to_submit != task_queue_.end() && to_submit->state == motdet_task_::task_state::done)
            {
                result_queue_.push_back({to_submit->timestamp, std::move(to_submit->result_conts)});
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
