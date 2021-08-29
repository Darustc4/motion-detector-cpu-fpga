#ifndef __MOTDET_CONTOUR_DETECTOR_HPP__
#define __MOTDET_CONTOUR_DETECTOR_HPP__

#include <cstdint>
#include "hls_stream.h"
#include "ap_int.h"

#include "motion_detector.hpp"

namespace motdet
{
    namespace imgutil
    {

        /**
         * @brief Detect connected components (contours) in a streamed binary image.
         * @param in Binary image. All values must be either 0 or 1 upon input. It is completely consumed.
         * @param conts The found contour boundign boxes without hierarchy.
         */
    	void connected_components(hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &in, hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &out);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_CONTOUR_DETECTOR_HPP__
