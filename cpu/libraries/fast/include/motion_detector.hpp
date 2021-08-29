#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <iostream>
#include <cstddef>
#include <vector>
#include <array>
#include <deque>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace motdet
{

    // Class definitions

    /**
     * @brief Represents an image (or video frame) as a flat structure.
     * @tparam T Type of pixel in the image. Cannot be bool, use unsigned char to store boolean values.
     */
    template <typename T> class Image
    {
    public:
        /**
         * @brief Construct a new Image object with the given width and height. Will fill all the pixels with the default value fill_value.
         * @param width Length of each row. >0.
         * @param height Row count. >0.
         * @param fill_value Value to use as default. Use {} for default initializer.
         */
        Image(const std::size_t width, const std::size_t height, const T fill_value):
            w_(width),
            h_(height),
            total_(width*height)
        {
            if(w_ == 0) throw std::invalid_argument("ERROR Constructor: width must be >0.");
            if(h_ == 0) throw std::invalid_argument("ERROR Constructor: height must be >0.");
            data_.resize(total_, fill_value);
        };

        /**
         * @brief Construct a new Image object from a given data vector. Each element of the vector represents a pixel.
         * @param init_data Vector to copy
         * @param width Lenght of each row in the inputted image. >0.
         */
        Image(const std::vector<T> &init_data, const std::size_t width):
            w_(width),
            h_(init_data.size()),
            total_(init_data.size())
        {
            if(w_ == 0) throw std::invalid_argument("ERROR Constructor: width must be >0.");
            h_ /= width;
            if(w_*h_ != init_data.size()) throw std::invalid_argument("ERROR Constructor: Invalid width for this vector length.");
            data_ = init_data;
        };

        Image() = default;
        Image(const Image &other) = default;
        Image(Image &&other) = default;

        // Operator Overload

        Image& operator=(const Image &other) = default;
        Image& operator=(Image &&other) = default;

        /**
         * @brief Returns an immutable T element located at idx.
         * @param idx Index to access.
         * @return Element located at idx.
         */
        inline const T& operator [](const std::size_t idx) const { return data_[idx]; };

        /**
         * @brief Returns a mutable T element located at idx.
         * @param idx Index to access.
         * @return Element located at idx.
         */
        inline T& operator [](const std::size_t idx) { return data_[idx]; };

        // Getters and Setters

        /**
         * @brief Get the width
         * @return std::size_t
         */
        inline std::size_t get_width() const  { return w_; };

        /**
         * @brief Get the height
         * @return std::size_t
         */
        inline std::size_t get_height() const { return h_; };

        /**
         * @brief Get the total pixels
         * @return std::size_t
         */
        inline std::size_t get_total() const  { return total_; };

        /**
         * @brief Get the internal data vector.
         * @return const std::vector<T>&
         */
        inline const std::vector<T>& get_data() const { return data_; }

        /**
         * @brief Set a new value for the internal data vector.
         * @param data Data vector, must be of the same type as the current one.
         * @param width Length of each row in the image.
         * @throw invalid_argument if width == 0 or the given width is not compatible with the given data vector.
         */
        inline void set_data(const std::vector<T>& data, const std::size_t width)
        {
            if(width == 0) throw std::invalid_argument("ERROR Constructor: width must be >0.");
            std::size_t height = data.size()/width;
            if(width*height != data.size()) throw std::invalid_argument("ERROR Constructor: Invalid width for this vector length.");

            w_ = width;
            h_ = height;
            total_ = data.size();
            data_ = data;
        }

        /**
         * @brief Set a new value for the internal data vector. But now for rvalues!
         * @param data Data vector, must be of the same type as the current one.
         * @param width Length of each row in the image.
         * @throw invalid_argument if width == 0 or the given width is not compatible with the given data vector.
         */
        inline void set_data(std::vector<T>&& data, const std::size_t width)
        {
            if(width == 0) throw std::invalid_argument("ERROR Constructor: width must be >0.");
            std::size_t height = data.size()/width;
            if(width*height != data.size()) throw std::invalid_argument("ERROR Constructor: Invalid width for this vector length.");

            w_ = width;
            h_ = height;
            total_ = data.size();
            data_ = std::move(data);
        }

    private:
        std::size_t w_, h_, total_;
        std::vector<T> data_;
    };


    /**
     * @brief Represents a bounding box on an image. Mainly used to return the position of the detected movement.
     */
    struct Contour
    {
        Contour(std::size_t bb_tl_x, std::size_t bb_tl_y, std::size_t bb_br_x, std::size_t bb_br_y):
            bb_tl_x(bb_tl_x),
            bb_tl_y(bb_tl_y),
            bb_br_x(bb_br_x),
            bb_br_y(bb_br_y)
        {}

        std::size_t bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour                                      */
        std::size_t bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour                                  */
    };


    /**
     * @brief Container for all the relevant info to return as a result when a frame is checked for movement.
     * @details Also contains the data_keep container that points at whatever data was sent as extra metadata when enqueing.
     */
    struct Detection
    {
        unsigned long long timestamp;            /**< The timestamp of the video the motion was detected from */
        bool has_detections;                     /**< True if motion has been detected                        */
        std::vector<Contour> detection_contours; /**< Contour of the detected movements                       */
        unsigned long long processing_time;      /**< Time it took the frame to be processed                  */

        std::shared_ptr<void> data_keep; /**< Will point at NULL if no data_keep was sent when enqueueing     */
    };

    /**
     * @brief Detects motion in a given grayscale frame, comparing against previous frames.
     */
    class Motion_detector{
    public:
        /**
         * @brief Constructor. Uses fps to set the rate at which the reference image is adjusted.
         * @details Creates a threaded motion detector object.
         * It uses a reference image internally to compare to and this reference is slowly interpolated with new frames to adapt to scenario changes.
         * If the update span is too high, precision loss might make the reference not update, 5 seconds is a good update span.
         * @param threads Number of threads to use for frame processing. Min 1. Reference updating is not 100% deterministic with >1 threads.
         * @param queue_size Amount of frames enqueued (waiting or processing). Any less than "threads" will cripple concurrency.
         * Recommended values is threads*2.
         * @param downsample_factor Reduce the size of the image for faster processing. 1 will not downsample. Must be >0.
         * @param frame_update_ratio Ratio at which the reference is updated. Closer to 0 is slower.
         * Calculate using the following formula: 1/(fps*seconds). The default used is fps = 30 and seconds = 5.
         * @throw invalid_argument if threads == 0, queue_size == 0, downsample_factor == 0, width < 10 or height < 10
         */
        Motion_detector(const std::size_t width, const std::size_t height, const std::size_t threads = 1, const std::size_t queue_size = 2, const unsigned int downsample_factor = 1, const float frame_update_ratio = 0.0067);

        Motion_detector() = delete;
        Motion_detector(const Motion_detector &other) = delete;
        Motion_detector(Motion_detector &&other) = delete;

        ~Motion_detector();

        // Operator Overload

        Motion_detector& operator=(const Motion_detector &other) = delete;
        Motion_detector& operator=(Motion_detector &&other) = delete;

        // Getters and Setters

        /**
         * @brief Get the width
         * @return std::size_t
         */
        inline std::size_t get_width() const { return w_; };

        /**
         * @brief Get the height
         * @return std::size_t
         */
        inline std::size_t get_height() const { return h_; };

        /**
         * @brief Get the total pixels
         * @return std::size_t
         */
        inline std::size_t get_total() const { return total_; };

        /**
         * @brief Get the maximum amount of tasks that can be queued.
         * @return std::size_t
         */
        inline std::size_t get_max_task_queue_size() const { return queue_size_; };

        /**
         * @brief Get the total amount of tasks stored in the queue, includes both frames not processed and those currently being processed.
         * @return std::size_t
         */
        inline std::size_t get_task_queue_size() const { return task_queue_.size(); };

        /**
         * @brief Get the total amount of completed tasks. Note a motion detector stores an infinite amount of completed tasks, make sure to collect them.
         * @return std::size_t
         */
        inline std::size_t get_result_queue_size() const { return result_queue_.size(); };

        // General Methods

        /**
         * @brief Will enqueue a frame to be processed by the image detector.
         * @param in Grayscale image to be processed wrapped in a smart pointer. Transfers ownership.
         * @param timestamp_millis Time in milliseconds of the frame being sent in.
         * @param blocking If true, will wait for queue to not be full, if false, will throw if queue is full.
         * @param data_keep Extra info to keep as "metadata" of the inputted frame, is returned as is upon result extraction.
         * An example usage would be to store the original RGB data of the frame here, so that it can be saved to disk later
         * if motion is detected.
         * @exception runtime_error if the queue is full and blocking is set to false. The input frame is lost forever.
         * @exception invalid_argument if the timestamp is older than one of the already enqueued frames.
         * @exception invalid_argument if the new frame has a different resolution from the one set in the constructor or is NULL.
         */
        void enqueue_frame(std::unique_ptr<Image<unsigned short>> in, unsigned long long timestamp_millis, bool blocking, std::shared_ptr<void> data_keep = {});

        /**
         * @brief Gets the contours detected in the oldest frame submitted to the motion detector.
         * @details Will only return successfully if the oldest frame submitted is finished, regardless of the completion state of other frames.
         * @return detection struct with the detected motion.
         * @exception runtime_error if the queue is empty and blocking is set to false.
         */
        Detection get_detection(bool blocking);

        /**
         * @brief Returns whether the oldest frame submitted has been processed. Thread safe method.
         * @return true if the oldest contours detected can be extracted safely with a non blocking get.
         * @return false if the contours are not yet ready to be returned.
         */
        bool is_frame_ready() const { std::unique_lock<std::mutex> locker(results_mutex_); return result_queue_.size() > 0; }

    private:

        std::size_t w_, h_, total_, downsampled_w_, downsampled_h_;
        std::size_t queue_size_;

        float frame_update_ratio_;
        unsigned int min_cont_area_, downsample_factor_;

        bool has_reference_ = false;
        Image<unsigned short> reference_;

        unsigned long long last_ref_update_time_, last_submitted_time_;

        /**
         * @brief Container for a motion detection job that is either pending for processing, is being processed or is done but not yet submitted.
         */
        struct Motdet_task_
        {
            enum class task_state : unsigned char { waiting, processing, done };

            unsigned long long timestamp, processing_time = 0;
            task_state state = task_state::waiting;

            std::unique_ptr<Image<unsigned short>> image;
            std::vector<Contour> result_conts;

            std::shared_ptr<void> data_keep;
        };

        std::size_t threads_;
        mutable std::mutex reference_mutex_, tasks_mutex_, results_mutex_;
        std::condition_variable tasks_full_cond_;      /**< Threads waiting for the task queue to not be empty. */
        std::condition_variable results_empty_cond_;   /**< Threads waiting for the oldest frame to be finished. */
        std::condition_variable no_processable_frame_; /**< Threads waiting for a processable frame to be in the tasks queue. */

        std::vector<std::thread> workers_container_;
        bool keep_workers_alive_ = true;

        std::deque<Motdet_task_> task_queue_; /**< Stores the queued tasks sent to the motion detector. From oldest to newest. */
        std::deque<Detection> result_queue_;  /**< Stores the resulting contorus detected. */

        void detect_motion_(std::size_t thread_id); /**< Executed by the worker threads on loop. */

        /**
         * @brief Checks if there is at least one task in the queue that can be processed. Does not lock mutex, so do it before calling the method.
         * @return true if there is a task that can be processed.
         * @return false if there are no tasks or all are either being processed or finished.
         */
        inline bool processable_frame_check_() noexcept
        {
            for(const Motdet_task_ &task : task_queue_) if(task.state == Motdet_task_::task_state::waiting) return true;
            return false;
        }
    };

    // Types

    typedef std::array<unsigned char, 3> rgb_pixel;

    // Functions

    /**
     * @brief Turns a color image to black and white (grayscale).
     * @details https://en.wikipedia.org/wiki/Luma_(video) adapted to 16b
     * @param in RGB image that will be converted. Must be completely initialized.
     * @param out 16b grayscale image that will be outputted.
     */
    void rgb_to_bw(const Image<rgb_pixel> &in, Image<unsigned short> &out);

    /**
     * @brief Turns an rgb uchar array to a black and white image (grayscale).
     * @details https://en.wikipedia.org/wiki/Luma_(video) adapted to 16b
     * @param in uchar C array that will be converted. Must be of length n_pix*3.
     * @param n_pix Number of elements present in array "in". An R-G-B triplet in "in" counts as 1 element.
     * @param out 16b grayscale image that will be outputted.
     */
    void uchar_to_bw(const unsigned char *in, const std::size_t n_pix, Image<unsigned short> &out);

} // namespace motdet

#endif // __MOTDET_MOTION_DETECTOR_HPP__
