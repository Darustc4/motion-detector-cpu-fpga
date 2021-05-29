#ifndef __MOTDET_CONTOUR_DETECTOR_HPP__
#define __MOTDET_CONTOUR_DETECTOR_HPP__

#include "motion_detector.hpp"

namespace motdet
{
    namespace imgutil
    {

        /**
         * @brief Detects contours in a binary image. The input image will be modified with the found contour IDs.
         * @details
         * Based on Topological Structural Analysis of Digitized Binary Images by Border Following, by Suzuki, S. and Abe, K. 1985. The resuling image from this function can be complex to understand, we recommend reading the original paper for a better understanding, or simply using the conts output which is simpler.
         * @param in Binary integer image. All values must be either 0 or 1. Will output these values changed to the id of the contours found (positive and negative depending on various factors).
         * @param conts The found contours with the contour hierarchy.
         * @param trim_borders if the input image has any value other than 0 in the outermost borders, it must be trimmed to apply this algorithm to it. Deactivate for better speed but make sure the input matrix has 0-pixel borders.
         */
        void contour_detection(Image<int> &in, std::vector<Contour> &conts, bool trim_borders = true);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_CONTOUR_DETECTOR_HPP__
