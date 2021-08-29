#ifndef __MOTDET_MOTION_DETECTOR_HPP__
#define __MOTDET_MOTION_DETECTOR_HPP__

#include <cstdint>
#include "hls_math.h"
#include "hls_stream.h"
#include "ap_int.h"

#define MOTDET_STREAM_DEPTH 1

#define ORIGINAL_WIDTH 1920 // (11b) 32
#define ORIGINAL_HEIGHT 1080 // (11b) 32
#define ORIGINAL_TOTAL 2073600 // (21b) 1024

// How to calculate: motdet_width = ceil( original_width/reduction_factor )
#define MOTDET_WIDTH 480 // (9b) 8
#define MOTDET_HEIGHT 270 // (9b) 8
#define MOTDET_TOTAL 129600 // (17b) 64

#define MOTDET_MAX_CONTOURS 1023
#define MOTDET_REDUCTION_FACTOR 4

namespace motdet
{
    const float motdet_frame_update_ratio = 0.0067;
    const ap_uint<15> motdet_threshold = 23500;
    const ap_uint<12> motdet_min_cont_area = MOTDET_TOTAL*0.004+5;

    // Stream depths

    const ap_uint<1> stream_depth = 1;

    struct Contour
    {
    	ap_uint<11> bb_tl_x, bb_tl_y; /**< Top left point of the bounding box of the Contour     */
    	ap_uint<11> bb_br_x, bb_br_y; /**< Bottom right point of the bounding box of the Contour */
    };

    struct Streamed_contour
    {
    	Contour contour;
    	bool stream_end;
    };

    struct Contour_package
    {
        Contour contours[MOTDET_MAX_CONTOURS];
        ap_uint<10> contour_count = 0;
    };

    struct Packed_pix
    {
    	ap_uint<16> pix[MOTDET_REDUCTION_FACTOR];
    };

} // namespace motdet

/**
 * @brief Detects motion in a sequence of grayscale frames. HLS TOP FUNCTION.
 * @param in grayscale streamed image where each pixel is represented by a 16b unsigned integer. The higher, the more intense white.
 * @param out Set of contours that have been detected as movement.
 */
void detect_motion(hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &in, hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &out);

#endif // __MOTDET_MOTION_DETECTOR_HPP__
