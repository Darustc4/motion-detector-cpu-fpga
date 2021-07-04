#ifndef __MOTDET_IMAGE_UTILS_HPP__
#define __MOTDET_IMAGE_UTILS_HPP__

#include "motion_detector.hpp"
#include "hls_stream.h"

#include <cstdint>

namespace motdet
{
    namespace imgutil
    {
        void set_reference(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in);

        /**
         * @brief Apply a 5x5 blurring filter to an image using a split kernel.
         * @param in Grayscale streamed image to blur.
         * @param out Grayscale streamed blurred image.
         */
        void gaussian_blur(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out);

        /**
         * @brief Resizes to a lower resolution by a given factor. Ignores floating point precision.
         * @param in Streamed image to resize, will use the sizes original_width and original_height.
         * @param out Resized image.
         */
        void downsample(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out);

        /**
         * @brief Updates the reference frame by interpolation. T pipes the input to the output so it can be chained with other functions.
         * @param in Streamed image that will be used to interpolate. The smaller motdet_frame_update_ratio, the less it will change the ref.
         * @param t_piped_out Exactly the same as "in".
         */
        void t_pipe_interpolate_reference(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &t_piped_out);

        /**
         * @brief Gets the absolute difference between an image and the reference image.
         * @param in Grayscale streamed image to subtract.
         * @param out Subtracted grayscale image, values are always positive.
         */
        void reference_subtraction(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out);

        /**
         * @brief Collapses all the values in a grayscale image to the states Culled 0 and Strong 1 depending on a threshold.
         * @param in Streamed image to collapse. Used motdet_threshold.
         * @param out Streamed image of the collapsed states.
         */
        void single_threshold(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &out);

        /**
         * @brief Takes a binary image (0 or 1) and dilates the 1-pixels.
         * @param in Streamed binary image to process.
         * @param out Streamed dilated binary image.
         */
        void dilation(hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &out);

    } // namespace imgutil
} // namespace motdet

#endif // __MOTDET_IMAGE_UTILS_HPP__
