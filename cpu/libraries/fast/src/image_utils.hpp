#ifndef __MOTDET_IMAGE_UTILS_HPP__
#define __MOTDET_IMAGE_UTILS_HPP__

#include "motion_detector.hpp"

#include <iostream>
#include <cstddef>    // std::size_t
#include <array>      // std::array
#include <functional> // std::function
#include <cmath>      // std::atan2 std::abs
#include <stack>      // std::stack

namespace motdet
{
    namespace imgutil
    {
        namespace detail
        {
            inline std::array<unsigned char, 5> gaussian_kernel_5_({ 16, 62, 99, 62, 16 });

            /**
             * @brief Fast square root approximation by Jim Ulery.
             * @details http://www.azillionmonkeys.com/qed/sqroot.html
             * @param val long integer to square root.
             * @return The approximation of the square root, no decimals.
             */
            inline unsigned long fast_sqrt_(unsigned long val);

            /**
             * @brief Blur a grayscale image vertically with a 5-length kernel. After this is applied to an image, an hline blur should be applied to complete the process.
             * @param in Grayscale image to be blurred.
             * @param out Blurred image.
             */
            void vline_blur(const Image<unsigned short> &in, Image<unsigned short> &out);

            /**
             * @brief Blur a grayscale image horizontally with a 5-length kernel. After this is applied to an image, a vline blur should be applied to complete the process.
             * @param in Grayscale image to be blurred.
             * @param out Blurred image.
             */
            void hline_blur(const Image<unsigned short> &in, Image<unsigned short> &out);

            /**
             * @brief Dilate a binary image vertically with a 3-length kernel. After this is applied to an image, an hline dilation should be applied to complete the process.
             * @param in Binary image to be dilated.
             * @param out Dilated image.
             */
            void vline_dilation(const Image<unsigned char> &in, Image<unsigned char> &out);

            /**
             * @brief Dilate a binary image horizontally with a 3-length kernel. After this is applied to an image, an hline dilation should be applied to complete the process.
             * @param in Binary image to be dilated.
             * @param out Dilated image.
             */
            void hline_dilation(const Image<unsigned char> &in, Image<unsigned char> &out);


        } // namespace detail

        /**
         * @brief Apply a 5x5 blurring filter to an image using a split kernel.
         * @param in Grayscale image to blur.
         * @param out Grayscale blurred image.
         */
        void gaussian_blur_filter(const Image<unsigned short> &in, Image<unsigned short> &out);

        /**
         * @brief Collapses all the values in a grayscale image to the states Culled 0, Strong 1 and Weak 2 depending on 2 thresholds.
         * @param in Image to collapse.
         * @param out Image of the collapsed states. A value that is between the 2 thresholds is set to Weak.
         * @param low_threshold Any value below this threshold is transformed to Culled.
         * @param high_threshold Any value equal or above this threshold is transformed to Strong. REQ: high_threshold > low_threshold.
         */
        void double_threshold(const Image<unsigned short> &in, Image<unsigned char> &out, const unsigned short low_threshold, const unsigned short high_threshold);

        /**
         * @brief Takes the output of a double threshold function and turns Weak pixel into either Strong or Culled.
         * @details Turns a Weak pixel into Strong if connected directly or indirectly to another Strong pixel, else culls it.
         * @param in Images with 3 possible values: Culled 0, Strong 1, Weak 2.
         * @param out Image with 2 possible values: Culled 0, Strong 1.
         */
        void hysteresis(const Image<unsigned char> &in, Image<unsigned char> &out);

        /**
         * @brief Creates an intermediate image between 2 given images. If ratio is 1 it will be equivalent to "to", and 0 will be equivalent to "from".
         * @param from Image that has more relevance the closer "ratio" is to 0.
         * @param to Image that has more relevance the closer "ratio" is to 1.
         * @param out Image with interpolated pixels.
         * @param ratio Selector for which input image has more relevance. [0-1]
         */
        void image_interpolation_and_sub(const Image<unsigned short> &from, const Image<unsigned short> &to, Image<unsigned short> &interpolated, Image<unsigned short> &subbed, const float ratio);

        /**
         * @brief Takes a binary image (0 or 1) and dilates the 1-pixels.
         * @param in Binary image to process.
         * @param out Dilated binary image.
         */
        void dilation(const Image<unsigned char> &in, Image<unsigned char> &out);

        /**
         * @brief Resizes to a lower resolution by a given factor. Ignores floating point precision.
         * @param in Image to resize, resolution must be at least "factor" in width and height.
         * @param out Resized image. Resolution must be ceil(in.w/factor) by ceil(in.h/factor)
         * @param factor Factor to resize the image, must be > 0.
         */
        void downsample(const Image<unsigned short> &in, Image<unsigned short> &out, std::size_t factor);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_IMAGE_UTILS_HPP__
