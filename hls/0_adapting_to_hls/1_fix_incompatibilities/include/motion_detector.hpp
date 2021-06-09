#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <cstdint>
#include <cmath>

// Define the resolution and reduction factor of the motion detector only if not defiend already.
// This allows compile time flexibility when importing the library not as a package, but as a header to compile.

#ifndef MOTDET_WIDTH
#define MOTDET_WIDTH 1920
#endif

#ifndef MOTDET_HEIGHT
#define MOTDET_HEIGHT 1080
#endif

#ifndef MOTDET_REDUCTION_FACTOR
#define MOTDET_REDUCTION_FACTOR 4
#endif

namespace motdet
{
    // Define the original and reduced resolutions, along other global variables to use in the library.

    const uint32_t original_width = MOTDET_WIDTH;
    const uint32_t original_height = MOTDET_HEIGHT;
    const uint32_t original_total = MOTDET_WIDTH*MOTDET_HEIGHT;

    const uint32_t motdet_width = std::ceil((float)original_width / MOTDET_REDUCTION_FACTOR);
    const uint32_t motdet_height = std::ceil((float)original_height / MOTDET_REDUCTION_FACTOR);
    const uint32_t motdet_total = motdet_width*motdet_height;

    const uint32_t max_contours = 100;

    // Class definitions

    /**
     * @brief Represents an image (or video frame) as a flat structure.
     * @details Will use stack memory if "__SYNTHESIS__" has been defined by the preprocessor. Else use dynamic memory with management.
     * @tparam T Type of pixel in the image. Cannot be bool, use uchar to store boolean values.
     * @tparam size Number of pixels to store in image. >0.
     */
    template <typename T, uint32_t size> class Image
    {
    public:

        Image() = delete;

        /**
         * @brief Construct a new Image object with the given width of each line.
         * @param width Length of each row. Must be size>=width>0 and size/width must have no remainder.
         */
        Image(const uint16_t width):
#ifndef __SYNTHESIS__
            data_(new T[size]{}),
#endif
            w_(width),
            h_(size/width)
        {};

        /**
         * @brief Construct a new Image object from a given data array. Each element of the array represents a pixel.
         * @param init_data Array to copy
         * @param width Lenght of each row in the inputted image. >0.
         */
        Image(const T (&init_data)[size], const uint16_t width):
#ifndef __SYNTHESIS__
            data_(new T[size]{}),
#endif
            w_(width),
            h_(size/width)
        {
            for(uint32_t i = 0; i < size; ++i) data_[i] = init_data[i];
        };

        /**
         * @brief Copy contructor.
         */
        Image(const Image &other)
#ifndef __SYNTHESIS__
            :data_(new T[size]{})
#endif
        {
            for(uint32_t i = 0; i < size; ++i) data_[i] = other.data_[i];
            w_ = other.w_;
            h_ = other.h_;
        };

        Image(Image &&other) = delete;

#ifndef __SYNTHESIS__
        /**
         * @brief Destroy the Image object. Only done if not synthetizing.
         */
        ~Image()
        {
            delete[] data_;
        }
#endif

        // Operator Overload

        /**
         * @brief Copy assignment.
         * @param other Other image to copy.
         * @return Image& Returs this object by reference for chaining.
         */
        Image& operator=(const Image &other)
        {
            for(uint32_t i = 0; i < size; ++i) data_[i] = other.data_[i];
            w_ = other.w_;
            h_ = other.h_;

            return *this;
        };

        Image& operator=(Image &&other) = delete;

        /**
         * @brief Returns an immutable T element located at idx.
         * @param idx Index to access.
         * @return Element located at idx.
         */
        inline const T& operator [](const uint32_t idx) const { return data_[idx]; };

        /**
         * @brief Returns a mutable T element located at idx.
         * @param idx Index to access.
         * @return Element located at idx.
         */
        inline T& operator [](const uint32_t idx) { return data_[idx]; };

        // Getters and Setters

        /**
         * @brief Get the width
         * @return uint16_t
         */
        inline uint16_t get_width() const  { return w_; };

        /**
         * @brief Get the height
         * @return uint16_t
         */
        inline uint16_t get_height() const { return h_; };

        /**
         * @brief Get the total pixels
         * @return uint32_t
         */
        inline uint32_t get_total() const  { return size; };

        /**
         * @brief Get the internal data array.
         * @return const T*
         */
        inline const T* get_data() const { return data_; }

    private:
        uint16_t w_, h_;

#ifndef __SYNTHESIS__
        T *data_;
#else
        T data_[size] = {};
#endif

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
            uint16_t bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour     */
            uint16_t bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour */
        };

        /**
         * @brief Container for all the relevant info to return as a result when a frame is checked for movement.
         * @details Also contains the data_keep container that points at whatever data was sent as extra metadata when enqueing.
         */
        struct Detection
        {
            uint32_t detection_count = 0;     /**< Amount of detected movement contours */
            Contour detections[max_contours]; /**< Contour of the detected movements    */
        };

        /**
         * @brief Constructor. Uses fps to set the rate at which the reference image is adjusted.
         * @details Creates a threaded motion detector object.
         * It uses a reference image internally to compare to and this reference is slowly interpolated with new frames to adapt to scenario changes.
         * If the update span is too high, precision loss might make the reference not update, 5 seconds is a good update span.
         * @param frame_update_ratio Ratio at which the reference is updated. Closer to 0 is slower.
         * Calculate using the following formula: 1/(fps*seconds). The recommended value is fps = 30 and seconds = 5, 0.0067.
         */
        Motion_detector(const float frame_update_ratio);

        Motion_detector(const Motion_detector &other) = delete;
        Motion_detector(Motion_detector &&other) = delete;

        // Operator Overload

        Motion_detector& operator=(const Motion_detector &other) = delete;
        Motion_detector& operator=(Motion_detector &&other) = delete;

        // General Methods

        void detect_motion(const Image<uint16_t, original_total> &in, Detection &out_detection);

    private:

        float frame_update_ratio_;
        uint32_t min_cont_area_;

        bool has_reference_ = false;
        Image<uint16_t, motdet_total> reference_;
    };


    /**
     * @brief Turns an rgb uchar array to a black and white image (grayscale).
     * @details https://en.wikipedia.org/wiki/Luma_(video) adapted to 16b
     * @param in uchar C array that will be converted. Must be of length n_pix*3.
     * @param n_pix Number of elements present in array "in". An R-G-B triplet in "in" counts as 1 element.
     * @param out 16b grayscale image that will be outputted.
     */
    void uchar_to_bw(const unsigned char *in, const uint32_t n_pix, Image<uint16_t, original_total> &out);

} // namespace motdet

#endif // __MOTDET_MOTION_DETECTOR_HPP__
