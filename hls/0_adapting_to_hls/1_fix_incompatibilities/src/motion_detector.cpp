#include "motion_detector.hpp"

#include "image_utils.hpp"
#include "contour_detector.hpp"

namespace motdet
{
    // Motion_detector implementation

    Motion_detector::Motion_detector(const float frame_update_ratio):
        frame_update_ratio_(frame_update_ratio),
        min_cont_area_(motdet_total*0.002+5),
        reference_(motdet_width)
    {}

    void Motion_detector::detect_motion(const Image<uint16_t, original_total> &in, Detection &out_detection)
    {
        out_detection.detection_count = 0;

        // Get the input image and downsample it, if needed.
        Image<uint16_t, motdet_total> downsampled_in(motdet_width);

        imgutil::downsample(in, downsampled_in);

        // If the motion detector has a reference frame, compare with it to check for motion. If not, make a new reference.
        if(has_reference_)
        {

            // Define all the container images for intermediate processing steps.
            Image<uint16_t, motdet_total> blur_image(motdet_width), sub_image(motdet_width);
            Image<uint8_t, motdet_total> thr_image(motdet_width), cnt_image(motdet_width), dil_image(motdet_width);

            // Blur the image to remove any noise that can result in false positives.
            imgutil::gaussian_blur_filter(downsampled_in, blur_image);

            // Subtract the blurred frame with the reference image. Leaving only the changes between frames.
            imgutil::image_subtraction(blur_image, reference_, sub_image);

            // Interpolate the blurred image and the reference frame to obtain a new reference.
            // Interpolation is done so that the reference can adapt to changing environment.
            imgutil::image_interpolation(reference_, blur_image, reference_, frame_update_ratio_);

            // Threshold the image so that any value below a certain number is ignored.
            // Using double threshold along with hysteresis for better results over single threshold.

            //imgutil::single_threshold(sub_image, cnt_image, 20000);

            imgutil::double_threshold(sub_image, thr_image, 8000, 25000);
            imgutil::hysteresis(thr_image, cnt_image);

            // Dilate the image so that the contours are better defined and with less holes.
            imgutil::dilation(cnt_image, dil_image);

            // Detect contours in the image. Any contour detected here is "movement".
            imgutil::Contour_package unfiltered_contours;

            imgutil::contour_detection(dil_image, unfiltered_contours);

            // Go over the detected contorus and discard any contour that is too small to be relevant.
            // Also scale the bounding box of the contour back to the original size before downscaling.
            for(uint32_t i = 0; i < unfiltered_contours.contour_count; ++i)
            {
                Contour &cont = unfiltered_contours.contours[i];
                uint32_t cont_area = (cont.bb_tl_x - cont.bb_br_x) * (cont.bb_tl_y - cont.bb_br_y) * MOTDET_REDUCTION_FACTOR;
                if(cont_area > min_cont_area_)
                {
                    cont.bb_tl_x *= MOTDET_REDUCTION_FACTOR;
                    cont.bb_br_x *= MOTDET_REDUCTION_FACTOR;
                    cont.bb_tl_y *= MOTDET_REDUCTION_FACTOR;
                    cont.bb_br_y *= MOTDET_REDUCTION_FACTOR;

                    out_detection.detections[out_detection.detection_count] = cont;
                    ++out_detection.detection_count;
                }
            }
        }
        else
        {
            // There is no reference, create a new one with the current input frame.
            imgutil::gaussian_blur_filter(downsampled_in, reference_);

            has_reference_ = true;
        }
    }

    // Other functions implementation

    void uchar_to_bw(const unsigned char *in, const uint32_t n_pix, Image<uint16_t, original_total> &out)
    {
        for(uint32_t i = 0; i < n_pix; ++i)
        {
            uint32_t mapped = i*3;
            out[i] = in[mapped] * 76.245 + in[mapped+1] * 149.685 + in[mapped+2] * 29.07;
        }
    }

} // namespace motdet
