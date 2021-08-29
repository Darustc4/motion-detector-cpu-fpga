#ifndef __MOTDET_CONTOUR_DETECTOR_HPP__
#define __MOTDET_CONTOUR_DETECTOR_HPP__

#include <cstdint>

#include "motion_detector.hpp"

namespace motdet
{
    namespace imgutil
    {

        struct Contour_package
        {
            Contour contours[max_contours];
            uint32_t contour_count = 0;
        };

        /**
         * @brief Detects contours in a binary image. The input image will be modified while processing.
         * @details
         * Based on Topological Structural Analysis of Digitized Binary Images by Border Following, by Suzuki, S. and Abe, K. 1985.
         * @param in Binary integer image. All values must be either 0 or 1 upon input. The output image is as defined in the paper.
         * @param conts The found contours without hierarchy.
         * @param trim_borders if the input image has any value other than 0 in the outermost borders, it must be trimmed to apply this algorithm to it.
         * Deactivate for better speed but make sure the input matrix has 0-pixel borders, else the function can hang in an infinite loop or segfault.
         */
        void contour_detection(Image<uint8_t, motdet_total> &in, Contour_package &conts, bool trim_borders = true);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_CONTOUR_DETECTOR_HPP__
