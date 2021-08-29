#include "motion_detector.hpp"

#include "image_utils.hpp"
#include "contour_detector.hpp"

namespace // Anonymous namespace
{
    bool has_reference = false;
} // Anonymous namespace

void detect_motion(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, motdet::Contour_package &out)
{
    using namespace motdet;

    hls::stream<uint16_t, MOTDET_STREAM_DEPTH> downsampled, blurred, t_piped_blurred, subbed;
    hls::stream<uint8_t, MOTDET_STREAM_DEPTH> thresholded, dilated;

    out.contour_count = 0;

    // Downsample the grayscale image.
    imgutil::downsample(in, downsampled);
    imgutil::gaussian_blur(downsampled, blurred);

    // If the motion detector has a reference frame, compare with it to check for motion. If not, make a new reference.
    if(has_reference)
    {
        // Interpolate the blurred image and the reference frame to obtain a new reference.
        // Interpolation is done so that the reference can adapt to changing environment.
        imgutil::t_pipe_interpolate_reference(blurred, t_piped_blurred);

        // Subtract the blurred frame with the reference image. Leaving only the changes between frames.
        imgutil::reference_subtraction(t_piped_blurred, subbed);

        // Threshold the image so that any value below a certain number is ignored.
        imgutil::single_threshold(subbed, thresholded);

        // Dilate the image so that the contours are better defined and with less holes.
        imgutil::dilation(thresholded, dilated);

        // Detect contours in the image. Any contour detected here is "movement".
        imgutil::connected_components(dilated, out);
    }
    else
    {
        has_reference = true;
        imgutil::set_reference(blurred);
    }
}

