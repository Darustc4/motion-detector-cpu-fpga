#include "motion_detector.hpp"

#include "image_utils.hpp"
#include "contour_detector.hpp"

void detect_motion(hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &in, hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &out)
{
#pragma HLS INTERFACE ap_fifo port=in
#pragma HLS INTERFACE ap_fifo port=out
#pragma HLS DATAFLOW
    using namespace motdet;

    hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> downsampled("downsampled");
    hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> blurred("blurred");
    hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> subbed("subbed");
    hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> thresholded("thresholded");
    hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> dilated("dilated");

	// Downsample the grayscale image.
	imgutil::downsample(in, downsampled);

	imgutil::gaussian_blur(downsampled, blurred);

    // Interpolate the blurred image and the reference frame to obtain a new reference.
    // Also subtract the blurred frame with the reference image. Leaving only the changes between frames.
    imgutil::apply_reference(blurred, subbed);

	// Threshold the image so that any value below a certain number is ignored.
	imgutil::single_threshold(subbed, thresholded);

	// Dilate the image so that the contours are better defined and with less holes.
	imgutil::dilation(thresholded, dilated);

	// Detect contours in the image. Any contour detected here is "movement".
	imgutil::connected_components(dilated, out);
}

