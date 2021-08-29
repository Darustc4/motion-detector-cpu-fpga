#ifndef __MOTDET_CONTOUR_DETECTOR_HPP__
#define __MOTDET_CONTOUR_DETECTOR_HPP__

#include <cstddef>
#include <iostream>
#include <vector>

#include "motion_detector.hpp"

namespace motdet
{
    namespace imgutil
    {

        /**
         * @brief Detects contours in a binary image. The input image will be modified with the found contour IDs.
         * @details
         * Based on Topological Structural Analysis of Digitized Binary Images by Border Following, by Suzuki, S. and Abe, K. 1985.
         * The algorithm has been modified to not collect topological information, so the resulting image will be simpler.
         * @param in Binary integer image. All values must be either 0 or 1 upon input. Upon output: 0 = No pixel, 1 = No border, 2 = Border, 3 = End of border.
         * @param trim_borders if the input image has any value other than 0 in the outermost borders, it must be trimmed to apply this algorithm to it.
         * Deactivate for better speed but make sure the input matrix has 0-pixel borders, else the function can hang in an infinite loop or segfault.
         * @return Vector of the bounding boxes of all the contours detected, regardless of their size.
         */
        std::vector<Contour> contour_detection(Image<unsigned char> &in, bool trim_borders = true);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_CONTOUR_DETECTOR_HPP__
