#include "motion_detector.hpp"

#include "image_utils.hpp"
#include "contour_detector.hpp"

void detect_motion(hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &in, hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &out)
{
#pragma HLS INTERFACE ap_fifo port=in
#pragma HLS INTERFACE ap_fifo port=out

    using namespace motdet;

    hls::stream<uint16_t, MOTDET_STREAM_DEPTH> downsampled("downsampled"), blurred("blurred"), t_piped_blurred("t-piped"), subbed("subbed");
    hls::stream<uint8_t, MOTDET_STREAM_DEPTH> thresholded("thresholded"), dilated("dilated");

    {
#pragma HLS DATAFLOW
		// Downsample the grayscale image.
    	imgutil::downsample(in, downsampled);

		imgutil::gaussian_blur(downsampled, blurred);
	}

    // Interpolate the blurred image and the reference frame to obtain a new reference.
	// Interpolation is done so that the reference can adapt to changing environment.
    imgutil::t_pipe_interpolate_reference(blurred, t_piped_blurred);

    // Subtract the blurred frame with the reference image. Leaving only the changes between frames.
	imgutil::reference_subtraction(t_piped_blurred, subbed);

	{
#pragma HLS DATAFLOW
		// Threshold the image so that any value below a certain number is ignored.
		imgutil::single_threshold(subbed, thresholded);

		// Dilate the image so that the contours are better defined and with less holes.
		imgutil::dilation(thresholded, dilated);

		// Detect contours in the image. Any contour detected here is "movement".
		imgutil::connected_components(dilated, out);
	}
}

