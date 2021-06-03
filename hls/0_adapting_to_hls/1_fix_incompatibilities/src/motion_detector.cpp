#include "motion_detector.hpp"

#include <iostream>

#include <chrono>
#include <stdexcept>
#include <cmath>

#include "image_utils.hpp"
#include "contour_detector.hpp"

namespace motdet
{
    // Motion_detector implementation

    Motion_detector::Motion_detector(const std::size_t width, const std::size_t height, const unsigned int downsample_factor, const float frame_update_ratio):
        w_(width),
        h_(height),
        total_(width*height),
        downsample_factor_(downsample_factor),
        frame_update_ratio_(frame_update_ratio),
        min_cont_area_(total_*0.002+5)
    {
        if (downsample_factor == 0) throw std::invalid_argument("ERROR Constructor: downsample_factor must be at least 1.");
        if (width      < 10)        throw std::invalid_argument("ERROR Constructor: width must be at least 10.");
        if (height     < 10)        throw std::invalid_argument("ERROR Constructor: height must be at least 10.");

        // The size requirements for the downsampled image are given by the function in image_utils.
        downsampled_w_ = std::ceil((float)w_ / downsample_factor);
        downsampled_h_ = std::ceil((float)h_ / downsample_factor);

        reference_ = Image<unsigned short>(downsampled_w_, downsampled_h_, {});
    }

    Motion_detector::Detection Motion_detector::detect_motion(const Image<unsigned short> &in)
    {
        if(in.get_total() != total_ || in.get_width() != w_) throw std::invalid_argument("ERROR Enqueue: Wrong resolution.");

        // It is assured by program logic that this frame will not be edited by another thread now. Begin processing.
        auto processing_time_start = std::chrono::high_resolution_clock::now();

        // Get the input image and downsample it, if needed.
        Image<unsigned short> downsampled_in(downsampled_w_, downsampled_h_, 0);

        if(downsample_factor_ > 1) imgutil::downsample(in, downsampled_in, downsample_factor_);
        else downsampled_in = std::move(in);

        Detection detection; // The result of the operation

        // If the motion detector has a reference frame, compare with it to check for motion. If not, make a new reference.
        if(has_reference_)
        {

            // Define all the container images for intermediate processing steps.
            Image<unsigned short> blur_image(downsampled_w_, downsampled_h_, 0), sub_image(downsampled_w_, downsampled_h_, 0), new_ref_image(downsampled_w_, downsampled_h_, 0);
            Image<unsigned char> thr_image(downsampled_w_, downsampled_h_, 0), cnt_image(downsampled_w_, downsampled_h_, 0);
            Image<int> dil_image(downsampled_w_, downsampled_h_, 0);

            // Blur the image to remove any noise that can result in false positives.
            imgutil::gaussian_blur_filter<unsigned short, unsigned short>(downsampled_in, blur_image);

            // Subtract the blurred frame with the reference image. Leaving only the changes between frames.
            imgutil::image_subtraction<unsigned short, unsigned short>(blur_image, reference_, sub_image);

            // Interpolate the blurred image and the reference frame to obtain a new reference.
            // Interpolation is done so that the reference can adapt to changing environment.
            imgutil::image_interpolation<unsigned short, unsigned short>(reference_, blur_image, new_ref_image, frame_update_ratio_);
            reference_ = std::move(new_ref_image);

            // Threshold the image so that any value below a certain number is ignored.
            // Using double threshold along with hysteresis for better results over single threshold.
            imgutil::double_threshold<unsigned short, unsigned char>(sub_image, thr_image, 8000, 25000);
            imgutil::hysteresis<unsigned char, unsigned char>(thr_image, cnt_image);

            // Dilate the image so that the contours are better defined and with less holes.
            imgutil::dilation<unsigned char, int>(cnt_image, dil_image);

            // Detect contours in the image. Any contour detected here is "movement".
            std::vector<Contour> unfiltered_contours;
            imgutil::contour_detection(dil_image, unfiltered_contours);

            // Go over the detected contorus and discard any contour that is too small to be relevant.
            // Also scale the bounding box of the contour back to the original size before downscaling.
            for(Contour &cont : unfiltered_contours)
            {
                unsigned int cont_area = (cont.bb_tl_x - cont.bb_br_x) * (cont.bb_tl_y - cont.bb_br_y) * downsample_factor_;
                if(cont_area > min_cont_area_)
                {
                    cont.bb_tl_x *= downsample_factor_;
                    cont.bb_br_x *= downsample_factor_;
                    cont.bb_tl_y *= downsample_factor_;
                    cont.bb_br_y *= downsample_factor_;
                    detection.detection_contours.push_back(std::move(cont));
                }
            }
        }
        else
        {
            // There is no reference, create a new one with the current input frame.
            imgutil::gaussian_blur_filter<unsigned short, unsigned short>(downsampled_in, reference_);

            has_reference_ = true;
        }

        auto processing_time_end = std::chrono::high_resolution_clock::now();
        detection.has_detections = detection.detection_contours.size() > 0;
        detection.processing_time = std::chrono::duration_cast<std::chrono::milliseconds>(processing_time_end - processing_time_start).count();

        return detection;
    }

    // Other functions implementation

    void uchar_to_bw(const unsigned char *in, const std::size_t n_pix, Image<unsigned short> &out)
    {
        for(std::size_t i = 0; i < n_pix; ++i)
        {
            std::size_t mapped = i*3;
            out[i] = in[mapped] * 76.245 + in[mapped+1] * 149.685 + in[mapped+2] * 29.07;
        }
    }

} // namespace motdet
