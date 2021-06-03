#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <iostream>
#include <cstddef>
#include <vector>

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
     * @brief Detects motion in a given grayscale frame, comparing against previous frames.
     */
    class Motion_detector{
    public:

        /**
         * @brief Represents a bounding box on an image. Mainly used to return the position of the detected movement.
         */
        struct Contour
        {
            int id;                       /**< Unique id of the contour within a Contours class                                       */
            int parent;                   /**< id of parent contour, 0 means top-level contour (no parent)                            */
            bool is_hole;                 /**< A contour can either surround a hole (0-pixels) or be an outline, surrounding 1-pixels */
            std::size_t n_pixels;         /**< Number of border pixels that compose this Contour                                      */
            std::size_t bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour                                      */
            std::size_t bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour                                  */
        };

        /**
         * @brief Container for all the relevant info to return as a result when a frame is checked for movement.
         * @details Also contains the data_keep container that points at whatever data was sent as extra metadata when enqueing.
         */
        struct Detection
        {
            bool has_detections;                     /**< True if motion has been detected                        */
            std::vector<Contour> detection_contours; /**< Contour of the detected movements                       */
            unsigned long long processing_time;      /**< Time it took the frame to be processed                  */
        };

        /**
         * @brief Constructor. Uses fps to set the rate at which the reference image is adjusted.
         * @details Creates a threaded motion detector object.
         * It uses a reference image internally to compare to and this reference is slowly interpolated with new frames to adapt to scenario changes.
         * If the update span is too high, precision loss might make the reference not update, 5 seconds is a good update span.
         * @param downsample_factor Reduce the size of the image for faster processing. 1 will not downsample. Must be >0.
         * @param frame_update_ratio Ratio at which the reference is updated. Closer to 0 is slower.
         * Calculate using the following formula: 1/(fps*seconds). The default used is fps = 30 and seconds = 5.
         * @throw invalid_argument if threads == 0, queue_size == 0, downsample_factor == 0, width < 10 or height < 10
         */
        Motion_detector(const std::size_t width, const std::size_t height, const unsigned int downsample_factor = 1, const float frame_update_ratio = 0.0067);

        Motion_detector() = delete;
        Motion_detector(const Motion_detector &other) = delete;
        Motion_detector(Motion_detector &&other) = delete;

        // Operator Overload

        Motion_detector& operator=(const Motion_detector &other) = delete;
        Motion_detector& operator=(Motion_detector &&other) = delete;

        // General Methods

        Detection detect_motion(const Image<unsigned short> &in);

    private:

        std::size_t w_, h_, total_, downsampled_w_, downsampled_h_;

        float frame_update_ratio_;
        unsigned int min_cont_area_, downsample_factor_;

        bool has_reference_ = false;
        Image<unsigned short> reference_;
    };


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
