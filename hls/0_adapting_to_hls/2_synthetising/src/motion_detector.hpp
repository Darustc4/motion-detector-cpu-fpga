#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <cstdint>
#include "hls_math.h"

namespace motdet
{
    const uint32_t original_width = 1920;
    const uint32_t original_height = 1080;
    const float motdet_frame_update_ratio = 0.0067;
    const uint32_t max_contours = 100;
    const uint32_t reduction_factor = 4;

    // How to calculate: motdet_width = ceil( original_width/reduction_factor )
    const uint32_t motdet_width = 480;
    const uint32_t motdet_height = 270;

    // Automatically set by constexpr magic
    const uint32_t original_total = original_width*original_height;
    const uint32_t motdet_total = motdet_width*motdet_height;
} // namespace motdet

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

struct Contour
{
    uint16_t bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour     */
    uint16_t bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour */
};

struct Detection
{
    uint32_t detection_count = 0;            /**< Amount of detected movement contours */
    Contour detections[motdet::max_contours]; /**< Contour of the detected movements    */
};

/**
 * @brief Detects motion in a sequence of grayscale frames. HLS TOP FUNCTION.
 * @param in grayscale image where each pixel is represented by a 16b unsigned integer. The higher, the more intense white.
 * @param out_detection Set of contours that have been detected as movement.
 */
void detect_motion(const Image<uint16_t, motdet::original_total> &in, Detection &out_detection);

#endif // __MOTDET_MOTION_DETECTOR_HPP__
