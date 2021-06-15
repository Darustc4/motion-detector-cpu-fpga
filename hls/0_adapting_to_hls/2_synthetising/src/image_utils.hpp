#ifndef __MOTDET_IMAGE_UTILS_HPP__
#define __MOTDET_IMAGE_UTILS_HPP__

#include "motion_detector.hpp"

#include <cstdint>

namespace motdet
{
    namespace imgutil
    {

        /**
         * @brief Apply a 5x5 blurring filter to an image using a traditional kernel. Slow but simple.
         * @param in Grayscale image to blur.
         * @param out Grayscale blurred image.
         */
        void gaussian_blur_filter(const Image<uint16_t, motdet_total> &in, Image<uint16_t, motdet_total> &out);

        /**
         * @brief Gets the absolute difference between 2 images (always positive). Useful for motion detection.
         * @param in1 Grayscale image 1 to subtract.
         * @param in2 Grayscale image 2 to subtract.
         * @param out Subtracted grayscale image, values are always positive.
         */
        void image_subtraction(const Image<uint16_t, motdet_total> &in1, const Image<uint16_t, motdet_total> &in2, Image<uint16_t, motdet_total> &out);

        /**
         * @brief Collapses all the values in a grayscale image to the states Culled 0, Strong 1 and Weak 2 depending on 2 thresholds.
         * @param in Image to collapse.
         * @param out Image of the collapsed states. A value that is between the 2 thresholds is set to Weak.
         * @param low_threshold Any value below this threshold is transformed to Culled.
         * @param high_threshold Any value equal or above this threshold is transformed to Strong. REQ: high_threshold > low_threshold.
         */
        void double_threshold(const Image<uint16_t, motdet_total> &in, Image<uint8_t, motdet_total> &out, const uint16_t low_threshold, const uint16_t high_threshold);

        /**
         * @brief Collapses all the values in a grayscale image to the states Culled 0 and Strong 1 depending on a threshold.
         * @param in Image to collapse.
         * @param out Image of the collapsed states.
         * @param threshold Any value below this threshold is set to Culled, the rest are Strong.
         */
        void single_threshold(const Image<uint16_t, motdet_total> &in, Image<uint8_t, motdet_total> &out, const uint16_t threshold);

        /**
         * @brief Takes the output of a double threshold function and turns Weak pixel into either Strong or Culled.
         * @details Turns a Weak pixel into Strong if connected directly or indirectly to another Strong pixel, else culls it.
         * @param in Images with 3 possible values: Culled 0, Strong 1, Weak 2.
         * @param out Image with 2 possible values: Culled 0, Strong 1.
         */
        void hysteresis(const Image<uint8_t, motdet_total> &in, Image<uint8_t, motdet_total> &out);

        /**
         * @brief Creates an intermediate image between 2 given images. If ratio is 1 it will be equivalent to "to", and 0 will be equivalent to "from".
         * @param from Image that has more relevance the closer "ratio" is to 0.
         * @param to Image that has more relevance the closer "ratio" is to 1.
         * @param out Image with interpolated pixels.
         * @param ratio Selector for which input image has more relevance. [0-1]
         */
        void image_interpolation(const Image<uint16_t, motdet_total> &from, const Image<uint16_t, motdet_total> &to, Image<uint16_t, motdet_total> &out, const float ratio);

        /**
         * @brief Takes a binary image (0 or 1) and dilates the 1-pixels. It interprets any value above 1 as 1. Below 0 is 0.
         * @param in Binary image to process.
         * @param out Dilated binary image.
         */
        void dilation(const Image<uint8_t, motdet_total> &in, Image<uint8_t, motdet_total> &out);

        /**
         * @brief Resizes to a lower resolution by a given factor. Ignores floating point precision.
         * @param in Image to resize, resolution must be at least "factor" in width and height.
         * @param out Resized image. Resolution must be ceil(in.w/factor) by ceil(in.h/factor)
         */
        void downsample(const Image<uint16_t, original_total> &in, Image<uint16_t, motdet_total> &out);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_IMAGE_UTILS_HPP__
