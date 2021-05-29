#ifndef __TEST_MOTDET_MOTION_DETECTOR_HPP__
#define __TEST_MOTDET_MOTION_DETECTOR_HPP__

#include "motion_detector.hpp"

namespace test
{
    namespace motion_detector
    {
        void test_all();

        bool test_image_constructor();
        bool test_image_assignment();
        bool test_image_indexing();
        bool test_image_getset();

        bool test_motion_detector_constructor();
        bool test_motion_detector_getset();
        bool test_motion_detector_detect_motion();

        bool test_rgb_to_bw();
        bool test_uchar_to_bw();
    } // namespace image_utils
} // namespace test

#endif // __TEST_MOTDET_MOTION_DETECTOR_HPP__
