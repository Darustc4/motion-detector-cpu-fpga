#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <cstdint>
#include "hls_math.h"
#include "hls_stream.h"

#ifndef MOTDET_STREAM_DEPTH
#define MOTDET_STREAM_DEPTH 2
#endif

namespace motdet
{
    const uint32_t original_width = 1920;
    const uint32_t original_height = 1080;
    const float motdet_frame_update_ratio = 0.0067;
    const uint32_t motdet_max_contours = 1023;
    const uint8_t motdet_reduction_factor = 4;

    const uint16_t motdet_threshold = 25000;

    // How to calculate: motdet_width = ceil( original_width/reduction_factor )
    const uint32_t motdet_width = 480;
    const uint32_t motdet_height = 270;

    // Automatically set by constexpr magic
    const uint32_t original_total = original_width*original_height;
    const uint32_t motdet_total = motdet_width*motdet_height;
    const uint32_t motdet_min_cont_area = motdet_total*0.004+5;

    struct Contour
    {
        uint16_t bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour     */
        uint16_t bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour */
    };

    struct Contour_package
    {
        Contour contours[motdet_max_contours];
        uint16_t contour_count = 0;
    };

    struct Packed_pix
    {
    	uint16_t pix[motdet_reduction_factor];
    };

} // namespace motdet

/**
 * @brief Detects motion in a sequence of grayscale frames. HLS TOP FUNCTION.
 * @param in grayscale streamed image where each pixel is represented by a 16b unsigned integer. The higher, the more intense white.
 * @param out Set of contours that have been detected as movement.
 */
void detect_motion(hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &in, motdet::Contour_package &out);

#endif // __MOTDET_MOTION_DETECTOR_HPP__
