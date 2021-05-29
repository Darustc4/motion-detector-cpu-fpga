#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <iostream>
#include <cstddef>
#include <vector>
#include <array>
#include <deque>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace motdet
{

    // Classes and structs

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

        inline const std::vector<T>& get_data() const { return data_; }

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

        // General Methods

        inline void print_data() const
        {
            for(std::size_t i = 0; i < h_; ++i)
            {
                for(std::size_t j = 0; j < w_; ++j)
                {
                    std::cout << std::setw(3) << (long long)data_[i*w_ + j] << ", ";
                }
                std::cout << std::endl;
            }
        }

    private:

        std::size_t w_, h_, total_;
        std::vector<T> data_;
    };


    /**
     * @brief A connected set of pixels with it's corresponding bounding box.
     */
    struct Contour{
        int id;                       /**< Unique id of the contour within a Contours class                                       */
        int parent;                   /**< id of parent contour, 0 means top-level contour (no parent)                            */
        bool is_hole;                 /**< A contour can either surround a hole (0-pixels) or be an outline, surrounding 1-pixels */
        std::size_t n_pixels;         /**< Number of border pixels that compose this Contour                                      */
        std::size_t bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour                                      */
        std::size_t bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour                                  */
    };


    /**
     * @brief Detects motion in a given grayscale frame, comparing against previous frames.
     */
    class Motion_detector{
    public:

        /**
         * @brief Constructor. Uses fps to set the rate at which the reference image is adjusted.
         * @details Creates a threaded motion detector object. It uses a reference image internally to compare to and this reference is slowly interpolated with new frames to adapt to scenario changes. If the update span is too high, precision loss might make the reference not update, 5 seconds is a good update span.
         * @param threads Number of threads to use for frame processing. Min 1. Reference updating is not 100% deterministic with >1 threads.
         * @param queue_size Amount of frames enqueued (waiting or processing). Any less than "threads" will cripple concurrency.
         * @param downsample_factor Reduce the size of the image for faster processing. 1 will not downsample. Must be >0.
         * @param frame_update_ratio Ratio at which the reference is updated. Closer to 0 is slower. Calculate: 1/(fps*seconds), default is fps 30, seconds 5.
         * @throw invalid_argument if threads == 0, queue_size == 0, width < 10 or height < 10
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
         * @param blocking If true, will wait for queue to not be full, if false, will throw if queue is full.
         * @exception runtime_error if the queue is full and blocking is set to false. The input frame is lost forever.
         * @exception invalid_argument if the timestamp is older than one of the already enqueued frames.
         * @exception invalid_argument if the new frame has a different resolution from the one set in the constructor.
         */
        void enqueue_frame(std::unique_ptr<Image<unsigned short>> in, unsigned long long timestamp_millis, bool blocking);

        /**
         * @brief Gets the contours detected in the oldest frame submitted to the motion detector.
         * @details Will only return successfully if the oldest frame submitted is finished, regardless of the completion state of other frames.
         * @return std::vector<Contour> Set of contours detected in the frame. If empty, no relevant motion was detected.
         * @exception runtime_error if the queue is empty and blocking is set to false.
         */
        std::pair<unsigned long long, std::vector<Contour>> get_detected_contours(bool blocking);

        /**
         * @brief Returns whether the oldest frame submitted has been processed. Thread safe method.
         * @return true if the oldest contours detected can be extracted safely with a non blocking get.
         * @return false if the contours are not yet ready to be returned.
         */
        bool is_frame_ready() const { std::unique_lock<std::mutex> locker(tasks_mutex_); return task_queue_.front().state == motdet_task_::task_state::done; }

        /**
         * @brief Will start saving grayscale debug images in the specified directory everytime detect_motion is called.
         * @param debug_dir Directory to save the images in. Must exist.
         */
        inline void save_debug_images(const std::string &debug_dir) { save_debug_images_ = true; debug_dir_ = std::filesystem::path(debug_dir); }

        /**
         * @brief Will stop saving debug images. If save_debug_images was not called, this method will not do anything.
         */
        inline void stop_debug_images() { save_debug_images_ = false; }

    private:

        std::size_t w_, h_, total_, downsampled_w_, downsampled_h_;
        std::size_t queue_size_;

        float frame_update_ratio_;
        unsigned int min_cont_area_, downsample_factor_;

        bool save_debug_images_ = false;
        std::filesystem::path debug_dir_;

        bool has_reference_ = false;
        Image<unsigned short> reference_;

        unsigned long long last_ref_update_time_, last_submitted_time_;

        struct motdet_task_
        {
            enum class task_state : unsigned char { waiting, processing, done };

            unsigned long long timestamp;
            task_state state = task_state::waiting;

            std::unique_ptr<Image<unsigned short>> image;
            std::vector<Contour> result_conts;
        };

        std::size_t threads_;
        mutable std::mutex reference_mutex_, tasks_mutex_, results_mutex_;
        std::condition_variable tasks_full_cond_;      /**< Threads waiting for the task queue to not be empty. */
        std::condition_variable results_empty_cond_;   /**< Threads waiting for the oldest frame to be finished. */
        std::condition_variable no_processable_frame_; /**< Threads waiting for a processable frame to be in the tasks queue. */

        std::vector<std::thread> workers_container_;
        bool keep_workers_alive_ = true;
        std::deque<motdet_task_> task_queue_; /**< Stores the queued tasks sent to the motion detector. From oldest to newest. */
        std::deque<std::pair<unsigned long long, std::vector<Contour>>> result_queue_; /**< Stores the resulting contorus detected. */

        void detect_motion_(std::size_t thread_id); /**< Executed by the worker threads on loop. */

        /**
         * @brief Checks if there is at least one task in the queue that can be processed. Does not lock mutex.
         * @return true if there is a task that can be processed.
         * @return false if there are no tasks or all are either being processed or finished.
         */
        inline bool processable_frame_check_() noexcept
        {
            for(const motdet_task_ &task : task_queue_) if(task.state == motdet_task_::task_state::waiting) return true;
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
