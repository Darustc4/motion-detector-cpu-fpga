#ifndef __MOTDET_IMAGE_UTILS_HPP__
#define __MOTDET_IMAGE_UTILS_HPP__

#include "motion_detector.hpp"

#include <iostream>
#include <cstddef>    // std::size_t
#include <array>      // std::array
#include <functional> // std::function
#include <cmath>      // std::atan2 std::abs
#include <stack>      // std::stack

/*
    This a template headers file. The implementation of the functions has been put in image_utils.ipp so that it resembled the structure
    of a hpp/cpp pair, but in the end an ipp file is simply a file that is appended to this file, very different from a cpp.

    Most of the functions in this module allow templated pixel types for the image inputs and outputs.
    This is done to allow easy conversions from different image types without needing to completely reallocate the image data.
    For example if you have an uchar image and need an int image for another function (i.e. contour detection)
    you template the function to input uchar and output int and it will be done.
    These templates require the type to be a BASIC TYPE, but if you make a custom type behave like a basic type, it will work too.
*/

namespace motdet
{
    namespace imgutil
    {
        namespace detail
        {
            inline float pi_ = 3.1416; /**< The constant PI. */
            inline unsigned char n_pixel_neighbor_ = 8; /**< Amount of pixels that surround a given pixel. In this case, 8. */

            inline std::array<float, 7> gaussian_kernel_5_({ 0.06136, 0.24477, 0.38775, 0.24477, 0.06136 });

            // 3x3 Horizontal sobel edge detection kernel.
            inline std::array<char, 9> sobel_h_kernel_3x3_({
                1,  0, -1,
                2,  0, -2,
                1,  0, -1
            });

            // 3x3 Vertical sobel edge detection kernel.
            inline std::array<char, 9> sobel_v_kernel_3x3_({
                1,   2,  1,
                0,   0,  0,
               -1,  -2, -1
            });

            /**
             * @brief Image kernel function. Find the median of 9 pixels (3x3 kernel).
             * @tparam IN_T must be a basic type of any bit length.
             * @tparam OUT_T must be a basic type of same or greater bit length than IN_T.
             * @param p Set of pixels of type IN_T to process.
             * @return The pixel from these 9 pixels that represents the median.
             */
            template <typename IN_T, typename OUT_T>
            OUT_T kernel_op_median_3x3_(std::array<IN_T, 9> &p);

            /**
             * @brief Image kernel function. Get the Gaussian blur value of a line kernel of length 5.
             * @tparam IN_T must be a basic type of any bit length. At max 64b.
             * @tparam OUT_T must be a basic type of same or greater bit length than IN_T.
             * @param p Set of pixels of type IN_T to process.
             * @return The value of the final blurred pixel.
             */
            template <typename IN_T, typename OUT_T>
            OUT_T kernel_op_gaussian_5_(const std::array<IN_T, 5> &p);

            /**
             * @brief Image kernel function. Detect horizontal edge value of a 3x3 kernel. The result can be negative.
             * @tparam IN_T must be a basic type of any bit length.
             * @tparam OUT_T must be a signed basic type. Minimum 2 bits longer than IN_T.
             * @param p Set of pixels of type IN_T to process.
             * @return The value of the horizontal edge.
             */
            template <typename IN_T, typename OUT_T>
            OUT_T kernel_op_sobel_h_3x3_(const std::array<IN_T, 9> &p);

            /**
             * @brief Image kernel function. Detect vertical edge value of a 3x3 kernel. The result can be negative.
             * @tparam IN_T must be a basic type of any bit length.
             * @tparam OUT_T must be a signed basic type. Minimum 2 bits longer than IN_T.
             * @param p Set of pixels of type IN_T to process.
             * @return The value of the vertical edge.
             */
            template <typename IN_T, typename OUT_T>
            OUT_T kernel_op_sobel_v_3x3_(const std::array<IN_T, 9> &p);

            /**
             * @brief Image kernel function. Returns 1 if any 1-pixel is in the kernel, else return 0.
             * @tparam IN_T must be a basic type of at least 1 bit (+1 for signed).
             * @tparam OUT_T must be a basic type of at least 1 bit (+1 for signed).
             * @tparam SIZE_K is the size of the kernel to use for dilation. Must be powers of odd numbers: 1, 9, 25, 49.
             * @param p Set of pixels of type IN_T to process.
             * @return The value of the dilated pixel.
             */
            template <typename IN_T, typename OUT_T, std::size_t SIZE_K>
            OUT_T kernel_op_dilation_(const std::array<IN_T, SIZE_K> &p);

            /**
             * @brief Image kernel function. Returns 0 if a 0-pixel is in the kernel, else return 1.
             * @tparam IN_T must be a basic type of at least 1 bit (+1 for signed).
             * @tparam OUT_T must be a basic type of at least 1 bit (+1 for signed).
             * @tparam SIZE_K is the size of the kernel to use for dilation. Must be powers of odd numbers: 1, 9, 25, 49.
             * @param p Set of pixels of type IN_T to process.
             * @return The value of the eroded pixel.
             */
            template <typename IN_T, typename OUT_T, std::size_t SIZE_K>
            OUT_T kernel_op_erosion_(const std::array<IN_T, SIZE_K> &p);

            /**
             * @brief Fast square root approximation by Jim Ulery.
             * @details http://www.azillionmonkeys.com/qed/sqroot.html
             * @param val long integer to square root.
             * @return The approximation of the square root, no decimals.
             */
            inline unsigned long fast_sqrt_(unsigned long val);

            /**
             * @brief Swaps the value of a and b. Used by pixel_sort().
             * @tparam IN_T Type of the parameters to swap. Must be a basic type.
             * @param a Parameter 1 to be swapped.
             * @param b Parameter 2 to be swapped.
             */
            template <typename IN_T>
            inline void pixel_swap_(IN_T &a, IN_T &b) { IN_T tmp(std::move(a)); a = b; b = tmp; }

            /**
             * @brief Swaps the value of a and b if a is bigger than b. Used by kernel_median_3x3().
             * @tparam IN_T Type of the parameters to sort. Must be a basic type.
             * @param a Parameter 1 to be compared and possibly swapped.
             * @param b Parameter 2 to be compared and possibly swapped.
             */
            template <typename IN_T>
            inline void pixel_sort_(IN_T &a, IN_T &b) { if (a > b) pixel_swap_<IN_T>(a, b); }

            /**
             * @brief Apply a generic square kernel to an image. The kernel is applied by a function that takes all the pixels in and array and outputs the pixel result.
             * @tparam IN_T must be a basic type of any bit length.
             * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
             * @tparam K_SIZE is the size of the kernel to extract around each pixel. Must be powers of odd numbers: 1, 9, 25, 49...
             * @param in Image to convolute.
             * @param out Image convoluted with the given kernel operator.
             * @param kernel_operator Function that receives the values of all the kernel elements and outputs the pixel value.
             */
            template<typename IN_T, typename OUT_T, std::size_t K_SIZE>
            void square_convolution(const Image<IN_T> &in, Image<OUT_T> &out, const std::function<OUT_T(std::array<IN_T, K_SIZE>&)> kernel_operator);

            /**
             * @brief Apply a vertical line kernel to an image. The kernel is applied by a function that takes all the pixels in and array and outputs the pixel result.
             * @tparam IN_T must be a basic type of any bit length.
             * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
             * @tparam K_SIZE is the size of the kernel to extract. Must be odd numbers: 1, 3, 5, 7...
             * @param in Image to convolute.
             * @param out Image convoluted with the given kernel operator.
             * @param kernel_operator Function that receives the values of all the kernel elements and outputs the pixel value.
             */
            template<typename IN_T, typename OUT_T, std::size_t K_SIZE>
            void vline_convolution(const Image<IN_T> &in, Image<OUT_T> &out, const std::function<OUT_T(std::array<IN_T, K_SIZE>&)> kernel_operator);

            /**
             * @brief Apply a horizontal line kernel to an image. The kernel is applied by a function that takes all the pixels in and array and outputs the pixel result.
             * @tparam IN_T must be a basic type of any bit length.
             * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
             * @tparam K_SIZE is the size of the kernel to extract. Must be odd numbers: 1, 3, 5, 7...
             * @param in Image to convolute.
             * @param out Image convoluted with the given kernel operator.
             * @param kernel_operator Function that receives the values of all the kernel elements and outputs the pixel value.
             */
            template<typename IN_T, typename OUT_T, std::size_t K_SIZE>
            void hline_convolution(const Image<IN_T> &in, Image<OUT_T> &out, const std::function<OUT_T(std::array<IN_T, K_SIZE>&)> kernel_operator);

        } // namespace detail

        /**
         * @brief Apply a 7x7 blurring filter to an image using a traditional kernel. Slow but simple.
         * @tparam IN_T Type of the pixels to be blurred. Must be a basic type of max 64b long.
         * @tparam OUT_T Type of the blurred pixels. Basic type of equal or greater bit length than IN_T.
         * @param in Grayscale image to blur.
         * @param out Grayscale blurred image.
         */
        template <typename IN_T, typename OUT_T>
        void gaussian_blur_filter(const Image<IN_T> &in, Image<OUT_T> &out);

        /**
         * @brief Apply a 3x3 median filter to an image. Useful for salt&pepper noise.
         * @tparam IN_T must be a basic type of any length.
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param in Grayscale image to process.
         * @param out Grayscale image with noise removed.
         */
        template <typename IN_T, typename OUT_T>
        void median_filter(const Image<IN_T> &in, Image<OUT_T> &out);

        /**
         * @brief Gets the absolute difference between 2 images (always positive). Useful for motion detection.
         * @tparam IN_T must be a basic type of any length.
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param in1 Grayscale image 1 to subtract.
         * @param in2 Grayscale image 2 to subtract.
         * @param out Subtracted grayscale image, values are always positive.
         */
        template <typename IN_T, typename OUT_T>
        void image_subtraction(const Image<IN_T> &in1, const Image<IN_T> &in2, Image<OUT_T> &out);

        /**
         * @brief Edge detection that produces a strictly binary image. 0 for no edge, 1 for edge.
         * @details https://towardsdatascience.com/canny-edge-detection-step-by-step-in-python-computer-vision-b49c3a2d8123
         * @tparam OUT_T must be a basic type of at least 1 bit.
         * @param in 8b grayscale image to process.
         * @param out Binary image of the edges. 1 = edge, 0 = no edge.
         * @param low_threshold Edges with strength below this value are ignored.
         * @param high_threshold Edges above this strength value are assured to appear in the final result. Strong edges.
         */
        template <typename OUT_T>
        void canny_edge_detection_8b(const Image<unsigned char> &in, Image<OUT_T> &out, const unsigned char low_threshold, const unsigned char high_threshold);


        /**
         * @brief Detect edges with magnitude (strength) and gradient (direction) in a grayscale picture.¡
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param in 8b grayscale image to process.
         * @param out_magnitude Grayscale image where a higer value means stronger edge.
         * @param out_gradient Grayscale image with the direction of the edges. 0 is 0 deg, 255 is 360 deg, it prioritizes memory efficiency to precision.
         */
        template<typename OUT_T>
        void sobel_edge_detection_8b(const Image<unsigned char> &in, Image<OUT_T> &out_magnitude, Image<unsigned char> &out_gradient);


        /**
         * @brief Reduces thickness of sobel edges in an image by removing non-essential points, picked out by edge direction analysis.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param in_magnitude Edges grayscale image, like the ones outputted by sobel_edge_detection().
         * @param in_gradient Grayscale image where 0 means 0 degrees and 255 means 360 degree direction.
         * @param out Grayscale image with reduces edge thickness.
         */
        template<typename IN_T, typename OUT_T>
        void non_max_suppression(const Image<IN_T> &in_magnitude, const Image<unsigned char> &in_gradient, Image<OUT_T> &out);

        /**
         * @brief Collapses all the values in a grayscale image to the states Culled 0, Strong 1 and Weak 2 depending on 2 thresholds.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of at least 2 bits (+1 for signed).
         * @param in Image to collapse.
         * @param out Image of the collapsed states. A value that is between the 2 thresholds is set to Weak.
         * @param low_threshold Any value below this threshold is transformed to Culled.
         * @param high_threshold Any value equal or above this threshold is transformed to Strong. REQ: high_threshold > low_threshold.
         */
        template <typename IN_T, typename OUT_T>
        void double_threshold(const Image<IN_T> &in, Image<OUT_T> &out, const IN_T low_threshold, const IN_T high_threshold);

        /**
         * @brief Collapses all the values in a grayscale image to the states Culled 0 and Strong 1 depending on a threshold.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of at least 1 bit (+1 for signed).
         * @param in Image to collapse.
         * @param out Image of the collapsed states.
         * @param threshold Any value below this threshold is set to Culled, the rest are Strong.
         */
        template <typename IN_T, typename OUT_T>
        void single_threshold(const Image<IN_T> &in, Image<OUT_T> &out, const IN_T threshold);

        /**
         * @brief Takes the output of a double threshold function and turns Weak pixel into either Strong or Culled.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of at least 1 bit (+1 for signed).
         * @details Turns a Weak pixel into Strong if connected directly or indirectly to another Strong pixel, else culls it.
         * @param in Images with 3 possible values: Culled 0, Strong 1, Weak 2.
         * @param out Image with 2 possible values: Culled 0, Strong 1.
         */
        template <typename IN_T, typename OUT_T>
        void hysteresis(const Image<IN_T> &in, Image<OUT_T> &out);

        /**
         * @brief Creates an intermediate image between 2 given images. If ratio is 1 it will be equivalent to "to", and 0 will be equivalent to "from".
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param from Image that has more relevance the closer "ratio" is to 0.
         * @param to Image that has more relevance the closer "ratio" is to 1.
         * @param out Image with interpolated pixels.
         * @param ratio Selector for which input image has more relevance. [0-1]
         */
        template <typename IN_T, typename OUT_T>
        void image_interpolation(const Image<IN_T> &from, const Image<IN_T> &to, Image<OUT_T> &out, const float ratio);

        /**
         * @brief Takes a binary image (0 or 1) and dilates the 1-pixels. It interprets any value above 1 as 1. Below 0 is 0.
         * @tparam IN_T must be a basic type of at least 1 bit (+1 for signed).
         * @tparam OUT_T must be a basic type of at least 1 bit (+1 for signed).
         * @param in Binary image to process.
         * @param out Dilated binary image.
         */
        template <typename IN_T, typename OUT_T>
        void dilation(const Image<IN_T> &in, Image<OUT_T> &out);

        /**
         * @brief Takes a binary image (0 or 1) and erodes the 1-pixels. It interprets any value above 1 as 1. Below 0 is 0.
         * @tparam IN_T must be a basic type of at least 1 bit (+1 for signed).
         * @tparam OUT_T must be a basic type of at least 1 bit (+1 for signed).
         * @param in Binary image to process.
         * @param out Eroded binary image.
         */
        template <typename IN_T, typename OUT_T>
        void erosion(const Image<IN_T> &in, Image<OUT_T> &out);

        /**
         * @brief Turn an image that only contains the values 0 and 1 to an image that only contains the values 0 and max_val.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of any bit length.
         * @param in Image that only contains the values 0 and 1. A value below 0 is considered 0 and above 1 is considered 1.
         * @param out Image with all the 1s in the input image turned to max_val.
         * @param max_val The value that will represent 1 in the output image.
         */
        template<typename IN_T, typename OUT_T>
        void reescale_pix_length(const Image<IN_T> &in, Image<OUT_T> &out, IN_T in_max_val, OUT_T out_max_val);

        /**
         * @brief Resizes to a lower resolution by a given factor. Ignores floating point precision.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param in Image to resize, resolution must be at least "factor" in width and height.
         * @param out Resized image. Resolution must be ceil(in.w/factor) by ceil(in.h/factor)
         * @param factor Factor to resize the image, must be > 0.
         */
        template<typename IN_T, typename OUT_T>
        void downsample(const Image<IN_T> &in, Image<OUT_T> &out, std::size_t factor);

        /**
         * @brief Resizes to a higher resolution by a given factor.
         * @tparam IN_T must be a basic type of any bit length.
         * @tparam OUT_T must be a basic type of equal or greater length than IN_T.
         * @param in Image to resize, can have any resolution.
         * @param out Resized image. Resolution must be in.w*factor by in.h*factor
         * @param factor Factor to resize the image, must be > 0.
         */
        template<typename IN_T, typename OUT_T>
        void upsample(const Image<IN_T> &in, Image<OUT_T> &out, std::size_t factor);

        /**
         * @brief Saves an 16b grayscale image to a given path.
         * @param path Where to store the image.
         * @param image An image that can contain any value from 0 to 65535.
         */
        inline void save_image_8b(const std::string &path, const Image<unsigned char> &image);

        // Include the file with the actual definitions for the headers we have declared above.
        #include "image_utils.ipp"

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_IMAGE_UTILS_HPP__
