
#include "image_utils.hpp"

namespace motdet
{
    namespace imgutil
    {
        namespace detail
        {

            inline unsigned long fast_sqrt_(unsigned long val)
            {
                unsigned long temp, g = 0, b = 0x8000, bshft = 15;

                do
                {
                    if (val >= (temp = (((g << 1) + b)<<bshft--)))
                    {
                        g += b;
                        val -= temp;
                    }
                }
                while (b >>= 1);

                return g;
            }

            void vline_blur(const Image<unsigned short> &in, Image<unsigned short> &out)
            {
                std::size_t current_pos;
                std::size_t height = in.get_height(), width = in.get_width();

                for(std::size_t i = 0; i < height; ++i)
                {
                    for(std::size_t j = 0; j < width; ++j)
                    {
                        current_pos = i * width + j;
                        unsigned long long sum = 0;

                        for(signed char ki = -2; ki <= 2; ++ki)
                        {
                            int real_ki = i + ki;

                            if(real_ki < 0) real_ki = 0;
                            else if (real_ki >= height) real_ki = height-1;

                            sum += in[real_ki*width + j] * gaussian_kernel_5_[ki + 2];
                        }
                        out[current_pos] = sum / 255;
                    }
                }
            }

            void hline_blur(const Image<unsigned short> &in, Image<unsigned short> &out)
            {
                std::size_t current_pos;
                std::size_t height = in.get_height(), width = in.get_width();

                for(std::size_t i = 0; i < height; ++i)
                {
                    for(std::size_t j = 0; j < width; ++j)
                    {
                        current_pos = i * width + j;
                        unsigned long long sum = 0;

                        for(int kj = -2; kj <= 2; ++kj)
                        {
                            int real_kj = j + kj;

                            if(real_kj < 0) real_kj = 0;
                            else if (real_kj >= width) real_kj = width-1;

                            sum += in[i*width + real_kj] * gaussian_kernel_5_[kj + 2];
                        }
                        // The kernel has been fully extracted, time to execute kernel operation.
                        out[current_pos] = sum/255;
                    }
                }
            }

            void vline_dilation(const Image<unsigned char> &in, Image<unsigned char> &out)
            {
                std::size_t current_pos;
                std::size_t height = in.get_height(), width = in.get_width();

                for(std::size_t i = 0; i < height; ++i)
                {
                    for(std::size_t j = 0; j < width; ++j)
                    {
                        current_pos = i * width + j;

                        if(in[current_pos] == 0)
                        {
                            if(i != 0 && in[current_pos - width] == 1) { out[current_pos] = 1; continue; }
                            if(i != height-1 && in[current_pos + width] == 1) { out[current_pos] = 1; continue; }
                            out[current_pos] = 0;
                        }
                        else out[current_pos] = 1;
                    }
                }
            }

            void hline_dilation(const Image<unsigned char> &in, Image<unsigned char> &out)
            {
                std::size_t current_pos;
                std::size_t height = in.get_height(), width = in.get_width();

                for(std::size_t i = 0; i < height; ++i)
                {
                    for(std::size_t j = 0; j < width; ++j)
                    {
                        current_pos = i * width + j;

                        if(in[current_pos] == 0)
                        {
                            if(j != 0 && in[current_pos - 1] == 1) { out[current_pos] = 1; continue; }
                            if(j != width-1 && in[current_pos + 1] == 1) { out[current_pos] = 1; continue; }
                            out[current_pos] = 0;
                        }
                        else out[current_pos] = 1;
                    }
                }
            }

        } // namespace detail

        void gaussian_blur_filter(const Image<unsigned short> &in, Image<unsigned short> &out)
        {
            std::size_t height = in.get_height(), width = in.get_width();
            Image<unsigned short> half_blurred(width, height, {});

            // An NxN gaussian blur can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
            // Using float as the intermediate pixel type to avoid detail loss between steps.

            detail::vline_blur(in, half_blurred     );
            detail::hline_blur(    half_blurred, out);
        }

        void double_threshold(const Image<unsigned short> &in, Image<unsigned char> &out, const unsigned short low_threshold, const unsigned short high_threshold)
        {
            std::size_t total = in.get_total();
            unsigned short val;

            for(int i = 0; i < total; ++i)
            {
                val = in[i];
                if     (val >= high_threshold) out[i] = 1; // Strong
                else if(val < low_threshold)   out[i] = 0; // Culled
                else                           out[i] = 2; // Weak
            }
        }

        void hysteresis(const Image<unsigned char> &in, Image<unsigned char> &out)
        {
            std::size_t stack_top = 0, current_pos, k_pos;
            std::stack<std::size_t> strong_pixel_stack;
            std::size_t height = in.get_height(), width = in.get_width();

            Image<unsigned char> visited_map(width, height, 0); // We need a way to check if a pixel has already been processed.

            // First iterate over image to set borders to 0 and collect all the strong edges into a stack.
            for(std::size_t i = 0; i < height; ++i)
            {
                for(std::size_t j = 0; j < width; ++j)
                {
                    current_pos = i * width + j;

                    // If edge of image, set to Culled and mark as visited.
                    if(j == 0 || j == width-1 || i == 0 || i == height-1)
                    {
                        out[current_pos] = 0;
                        visited_map[current_pos] = 1;
                    }
                    // If current position is a strong edge, add to the stack to check later
                    else if(in[current_pos] == 1) strong_pixel_stack.push(current_pos);
                }
            }

            // Once all the strong edges are collected, analyze them for neighboring weak edges that can be set as strong.
            while(!strong_pixel_stack.empty())
            {
                current_pos = strong_pixel_stack.top();
                strong_pixel_stack.pop();

                out[current_pos] = 1;

                for(signed char ki = -1; ki < 2; ++ki)
                {
                    for(signed char kj = -1; kj < 2; ++kj)
                    {
                        k_pos = current_pos + width*ki + kj;
                        if(!visited_map[k_pos] && in[k_pos] == 2) strong_pixel_stack.push(k_pos);
                        visited_map[k_pos] = true;
                    }
                }
            }
        }

        void image_interpolation_and_sub(const Image<unsigned short> &from, const Image<unsigned short> &to, Image<unsigned short> &interpolated, Image<unsigned short> &subbed, const float ratio)
        {
            std::size_t total = interpolated.get_total();

            for(std::size_t i = 0; i < total; ++i)
            {
                unsigned short from_pix = from[i];
                unsigned short to_pix = to[i];
                int sub = to_pix - from_pix;

                subbed[i] = std::abs(sub);
                interpolated[i] = from_pix + ratio * sub; // Simplified from equation: from[i]*(1-ratio) + to[i]*ratio
            }
        }


        void dilation(const Image<unsigned char> &in, Image<unsigned char> &out)
        {
            std::size_t height = in.get_height(), width = in.get_width();
            Image<unsigned char> half_dilated(width, height, {});

            detail::vline_dilation(in, half_dilated);
            detail::hline_dilation(    half_dilated, out);
        }

        void downsample(const Image<unsigned short> &in, Image<unsigned short> &out, std::size_t factor)
        {
            std::size_t in_height = in.get_height(), in_width = in.get_width();
            std::size_t out_height = out.get_height(), out_width = out.get_width();

            std::size_t excess_height = in_height%factor, excess_width = in_width%factor;
            std::size_t iter_height = out_height, iter_width = out_width;

            if(excess_height > 0) --iter_height;
            if(excess_width > 0) --iter_width;

            // The image will be analyzed with sampler boxes of size factorxfactor.
            // The image is divided into 4 sections.
            // 1.- Area that fits within the sampler box matrix
            // 2.- Excess area on the side (w) that does not fit in the sampler matrix.
            // 3.- Excess area at the botton (h) that does not fit in the sampler matrix.
            // 4.- Excess corner at the bottom right of the image.

            unsigned int sampler_divisor = factor*factor;
            unsigned int sampler_divisor_excessw = factor*excess_width;
            unsigned int sampler_divisor_excessh = factor*excess_height;
            unsigned int sampler_divisor_excesswh = excess_width*excess_height;
            long long sampler_accumulator;

            for(std::size_t i = 0; i < iter_height; ++i)
            {
                std::size_t sampler_i = i*factor;
                for(std::size_t j = 0; j < iter_width; ++j)
                {
                    std::size_t sampler_j = j*factor;
                    std::size_t sampler_pos = sampler_i*in_width + sampler_j;
                    sampler_accumulator = 0;

                    for(std::size_t box_i = 0; box_i < factor; ++box_i)
                    {
                        std::size_t box_i_dis = box_i*in_width;
                        for(std::size_t box_j = 0; box_j < factor; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }
                    out[i*out_width + j] = sampler_accumulator / sampler_divisor;
                }

                if(excess_width)
                {
                    // Now process the excess width for the corresponding sampler row

                    std::size_t sampler_pos = (sampler_i+1)*in_width - excess_width;
                    sampler_accumulator = 0;

                    for(std::size_t box_i = 0; box_i < factor; ++box_i)
                    {
                        std::size_t box_i_dis = box_i*in_width;
                        for(std::size_t box_j = 0; box_j < excess_width; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }
                    out[(i+1)*out_width - 1] = sampler_accumulator / sampler_divisor_excessw;
                }
            }

            if(excess_height)
            {
                // Now process the excess height of the image, the ramaining excess bottom

                std::size_t sampler_i = (in_height-excess_width+1)*in_width;
                for(std::size_t j = 0; j < iter_width; ++j)
                {
                    std::size_t sampler_pos = sampler_i + j*factor;
                    sampler_accumulator = 0;

                    for(std::size_t box_i = 0; box_i < excess_height; ++box_i)
                    {
                        std::size_t box_i_dis = box_i*in_width;
                        for(std::size_t box_j = 0; box_j < factor; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }
                    out[(out_height-1)*out_width + j] = sampler_accumulator / sampler_divisor_excessh;
                }

                if(excess_width)
                {
                    // Process the last excess corner at the bottom right
                    std::size_t sampler_pos = ((in_height-excess_width+1)*in_width) + in_width-excess_width;
                    sampler_accumulator = 0;

                    for(std::size_t box_i = 0; box_i < excess_height; ++box_i)
                    {
                        std::size_t box_i_dis = box_i*in_width;
                        for(std::size_t box_j = 0; box_j < excess_width; ++box_j)
                        {
                            sampler_accumulator += in[sampler_pos + box_i_dis + box_j];
                        }
                    }

                    out[(out_height-1)*out_width + out_width - 1] = sampler_accumulator / sampler_divisor_excesswh;
                }
            }
        }
    } // namespace imgutil
} // namespace motdet
