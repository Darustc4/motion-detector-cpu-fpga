#ifndef __TEST_MOTDET_IMAGE_UTILS_HPP__
#define __TEST_MOTDET_IMAGE_UTILS_HPP__

#include "motion_detector.hpp"
#include "image_utils.hpp"

namespace test
{
    namespace image_utils
    {
        void test_all();

        bool test_detail_vline_convolution();
        bool test_detail_hline_convolution();
        bool test_gaussian_blur_filter();
        bool test_image_subtraction();
        bool test_double_threshold();
        bool test_single_threshold();
        bool test_hysteresis();
        bool test_image_interpolation();
        bool test_dilation();
        bool test_downsample();
    } // namespace image_utils
} // namespace test

#endif // __TEST_MOTDET_IMAGE_UTILS_HPP__
