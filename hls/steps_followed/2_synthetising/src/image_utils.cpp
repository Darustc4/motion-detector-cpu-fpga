#include "image_utils.hpp"

namespace motdet
{
    namespace imgutil
    {
        namespace
        {
            float gaussian_kernel[5] = { 0.06136, 0.24477, 0.38775, 0.24477, 0.06136 };

            void gaussian_blur_filter_vline(const Image<uint16_t, motdet_total> &in, Image<uint16_t, motdet_total> &out)
            {
                uint32_t current_pos;

                for(uint16_t i = 0; i < motdet_height; ++i)
                {
                    uint32_t i_pos = i * motdet_width;

                    for(uint16_t j = 0; j < motdet_width; ++j)
                    {
                        current_pos = i_pos + j;
                        uint32_t blur_sum = 0;

                        for(int8_t ki = -2; ki <= 2; ++ki)
                        {
                            int16_t real_ki = i + ki;

                            if(real_ki < 0) real_ki = 0;
                            else if (real_ki >= motdet_height) real_ki = motdet_height-1;

                            blur_sum += in[real_ki*motdet_width + j] * gaussian_kernel[ki + 2];
                        }
                        out[current_pos] = blur_sum;
                    }
                }
            }

            void gaussian_blur_filter_hline(const Image<uint16_t, motdet_total> &in, Image<uint16_t, motdet_total> &out)
            {
                uint32_t current_pos;

                for(uint16_t i = 0; i < motdet_height; ++i)
                {
                    uint32_t i_pos = i * motdet_width;
                    for(uint16_t j = 0; j < motdet_width; ++j)
                    {
                        current_pos = i_pos + j;
                        uint32_t blur_sum = 0;

                        for(int8_t kj = -2; kj <= 2; ++kj)
                        {
                            int16_t real_kj = j + kj;

                            if(real_kj < 0) real_kj = 0;
                            else if (real_kj >= motdet_width) real_kj = motdet_width-1;

                            blur_sum += in[i*motdet_width + real_kj] * gaussian_kernel[kj + 2];
                        }
                        out[current_pos] = blur_sum;
                    }
                }
            }

            void dilation_filter_vline(const Image<uint8_t, motdet_total> &in, Image<uint8_t, motdet_total> &out)
            {
                uint32_t current_pos;

                for(uint16_t i = 0; i < motdet_height; ++i)
                {
                    uint32_t i_pos = i * motdet_width;

                    for(uint16_t j = 0; j < motdet_width; ++j)
                    {
                        current_pos = i_pos + j;

                        if(!in[current_pos])
                        {
                            out[current_pos] = 0;
                            for(uint8_t ki = 0; ki < 3; ++ki)
                            {
                                int16_t real_ki = i + ki - 1;

                                if(real_ki < 0) continue;
                                if (real_ki >= motdet_height) continue;

                                if(in[real_ki*motdet_width + j] >= 1) out[current_pos] = 1;
                            }
                        }
                        else out[current_pos] = 1;
                    }
                }
            }

            void dilation_filter_hline(const Image<uint8_t, motdet_total> &in, Image<uint8_t, motdet_total> &out)
            {
                uint32_t current_pos;

                for(uint16_t i = 0; i < motdet_height; ++i)
                {
                    uint32_t i_pos = i * motdet_width;

                    for(uint16_t j = 0; j < motdet_width; ++j)
                    {
                        current_pos = i_pos + j;

                        if(!in[current_pos])
                        {
                            out[current_pos] = 0;
                            for(uint8_t kj = 0; kj < 3; ++kj)
                            {
                                int16_t real_kj = j + kj - 1;

                                if(real_kj < 0) continue;
                                if (real_kj >= motdet_width) continue;

                                if(in[i*motdet_width + real_kj] >= 1) out[current_pos] = 1;
                            }
                        }
                        else out[current_pos] = 1;
                    }
                }
            }
        }

        void gaussian_blur_filter(const Image<uint16_t, motdet_total> &in, Image<uint16_t, motdet_total> &out)
        {
            Image<uint16_t, motdet_total> half_blurred(motdet_width);

            // An NxN gaussian blur can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
            gaussian_blur_filter_vline(in, half_blurred);
            gaussian_blur_filter_hline(half_blurred, out);
        }

        void image_subtraction(const Image<uint16_t, motdet_total> &in1, const Image<uint16_t, motdet_total> &in2, Image<uint16_t, motdet_total> &out)
        {
            for(uint32_t i = 0; i < motdet_total; ++i) out[i] = hls::abs(in1[i] - in2[i]);
        }

        void double_threshold(const Image<uint16_t, motdet_total> &in, Image<uint8_t, motdet_total> &out, const uint16_t low_threshold, const uint16_t high_threshold)
        {
            uint16_t val;

            for(uint32_t i = 0; i < motdet_total; ++i)
            {
                val = in[i]; // HLS loves ternary operators, use them over conditionals if possible for better synthetization.
                out[i] = val >= high_threshold ? 1 : val < low_threshold ? 0 : 2;
            }
        }

        void single_threshold(const Image<uint16_t, motdet_total> &in, Image<uint8_t, motdet_total> &out, const uint16_t threshold)
        {
            for(uint32_t i = 0; i < motdet_total; ++i) out[i] = in[i] >= threshold ? 1 : 0;
        }

        void hysteresis(const Image<uint8_t, motdet_total> &in, Image<uint8_t, motdet_total> &out)
        {
            uint32_t stack_top = 0, current_pos, k_pos;
            uint32_t strong_pixel_stack[motdet_total]; // Lets assume the worst case scenary, everything is an edge.

            Image<uint8_t, motdet_total> visited_map(motdet_width); // We need a way to check if a pixel has already been processed.

            // First iterate over image to set borders to 0 and collect all the strong edges into a stack.
            for(uint16_t i = 0; i < motdet_height; ++i)
            {
                uint32_t pos_i = i * motdet_width;
                for(uint16_t j = 0; j < motdet_width; ++j)
                {
                    current_pos = pos_i + j;

                    // If edge of image, set to Culled and mark as visited.
                    if(j == 0 || j == motdet_width-1 || i == 0 || i == motdet_height-1)
                    {
                        out[current_pos] = 0;
                        visited_map[current_pos] = 1;
                    }
                    // If current position is a strong edge, add to the stack to check later
                    else if(in[current_pos] == 1)
                    {
                        strong_pixel_stack[stack_top++] = current_pos;
                    }
                }
            }

            // Once all the strong edges are collected, analyze them for neighboring weak edges that can be set as strong.
            while(stack_top > 0)
            {
                current_pos = strong_pixel_stack[--stack_top];

                out[current_pos] = 1;

                for(int8_t ki = -1; ki < 2; ++ki)
                {
                    int16_t pos_ki = motdet_width*ki;
                    for(int8_t kj = -1; kj < 2; ++kj)
                    {
                        k_pos = current_pos + pos_ki + kj;
                        if(!visited_map[k_pos] && in[k_pos] == 2) strong_pixel_stack[stack_top++] = k_pos;
                        visited_map[k_pos] = true;
                    }
                }
            }
        }

        void image_interpolation(const Image<uint16_t, motdet_total> &from, const Image<uint16_t, motdet_total> &to, Image<uint16_t, motdet_total> &out, const float ratio)
        {
            for(uint32_t i = 0; i < motdet_total; ++i)
            {
                out[i] = from[i] + ratio * (to[i] - from[i]); // Simplified from equation: from[i]*(1-ratio) + to[i]*ratio
            }
        }

        void dilation(const Image<uint8_t, motdet_total> &in, Image<uint8_t, motdet_total> &out)
        {
            Image<uint8_t, motdet_total> half_dilated(motdet_width);

            dilation_filter_vline(in, half_dilated);
            dilation_filter_hline(half_dilated, out);
        }


        void downsample(const Image<uint16_t, original_total> &in, Image<uint16_t, motdet_total> &out)
        {
            uint8_t factor = reduction_factor;
            uint8_t excess_height = original_height%factor, excess_width = original_width%factor;
            uint16_t iter_height = motdet_height, iter_width = motdet_width;

            if(excess_height > 0) --iter_height;
            if(excess_width > 0) --iter_width;

            // The image will be analyzed with sampler boxes of size factorxfactor.
            // The image is divided into 4 sections.
            // 1.- Area that fits within the sampler box matrix
            // 2.- Excess area on the side (w) that does not fit in the sampler matrix.
            // 3.- Excess area at the botton (h) that does not fit in the sampler matrix.
            // 4.- Excess corner at the bottom right of the image.

            uint8_t sampler_divisor = factor*factor;
            uint8_t sampler_divisor_excessw = factor*excess_width;
            uint8_t sampler_divisor_excessh = factor*excess_height;
            uint8_t sampler_divisor_excesswh = excess_width*excess_height;
            uint32_t sampler_accumulator;

            for(uint16_t i = 0; i < iter_height; ++i)
            {
                uint32_t sampler_i = i*factor;
                for(uint16_t j = 0; j < iter_width; ++j)
                {
                    uint32_t sampler_j = j*factor;
                    uint32_t sampler_pos = sampler_i*original_width + sampler_j;
                    sampler_accumulator = 0;

                    for(uint8_t box_i = 0; box_i < factor; ++box_i)
                    {
                        uint32_t box_i_dis = box_i*original_width;
                        for(uint8_t box_j = 0; box_j < factor; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }
                    out[i*motdet_width + j] = sampler_accumulator / sampler_divisor;
                }

                if(excess_width)
                {
                    // Now process the excess width for the corresponding sampler row

                    uint32_t sampler_pos = (sampler_i+1)*original_width - excess_width;
                    sampler_accumulator = 0;

                    for(uint8_t box_i = 0; box_i < factor; ++box_i)
                    {
                        uint32_t box_i_dis = box_i*original_width;
                        for(uint8_t box_j = 0; box_j < excess_width; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }
                    out[(i+1)*motdet_width - 1] = sampler_accumulator / sampler_divisor_excessw;
                }
            }

            if(excess_height)
            {
                // Now process the excess height of the image, the ramaining excess bottom

                uint32_t sampler_i = (original_height-excess_width+1)*original_width;
                for(uint16_t j = 0; j < iter_width; ++j)
                {
                    uint32_t sampler_pos = sampler_i + j*factor;
                    sampler_accumulator = 0;

                    for(uint8_t box_i = 0; box_i < excess_height; ++box_i)
                    {
                        uint32_t box_i_dis = box_i*original_width;
                        for(uint8_t box_j = 0; box_j < factor; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }
                    out[(motdet_height-1)*motdet_width + j] = sampler_accumulator / sampler_divisor_excessh;
                }

                if(excess_width)
                {
                    // Process the last excess corner at the bottom right
                    uint32_t sampler_pos = ((original_height-excess_width+1)*original_width) + original_width-excess_width;
                    sampler_accumulator = 0;

                    for(uint16_t box_i = 0; box_i < motdet_height; ++box_i)
                    {
                        uint32_t box_i_dis = box_i*original_width;
                        for(uint8_t box_j = 0; box_j < excess_width; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }

                    out[(motdet_height)*motdet_width - 1] = sampler_accumulator / sampler_divisor_excesswh;
                }
            }
        }
    } // namespace imgutil
} // namespace motdet
