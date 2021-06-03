
namespace detail
{
    template <typename IN_T, typename OUT_T>
    OUT_T kernel_op_gaussian_5_(const std::array<IN_T, 5> &p)
    {
        double total = 0;
        for(std::size_t i = 0; i < 5; ++i) total += p[i] * gaussian_kernel_5_[i];
        return total;
    }

    template <typename IN_T, typename OUT_T, std::size_t SIZE_K>
    OUT_T kernel_op_dilation_(const std::array<IN_T, SIZE_K> &p)
    {
        for(const IN_T &i: p)
        {
            if (i >= 1) return 1;
        }
        return 0;
    }

    template<typename IN_T, typename OUT_T, std::size_t K_SIZE>
    void vline_convolution(const Image<IN_T> &in, Image<OUT_T> &out, const std::function<OUT_T(std::array<IN_T, K_SIZE>&)> kernel_operator)
    {
        std::size_t current_pos;
        std::size_t height = in.get_height(), width = in.get_width();
        int kernel_radius;

        if(K_SIZE % 2 == 0) throw std::invalid_argument("Kernel is of invalid size.");

        kernel_radius = K_SIZE >> 1;

        std::array<IN_T, K_SIZE> extracted_kernel;

        for(std::size_t i = 0; i < height; ++i)
        {
            for(std::size_t j = 0; j < width; ++j)
            {
                current_pos = i * width + j;

                for(int ki = -kernel_radius; ki <= kernel_radius; ++ki)
                {
                    int real_ki = i + ki;

                    if(real_ki < 0) real_ki = 0;
                    else if (real_ki >= height) real_ki = height-1;

                    // Fill in the kernel that will be passed to the function with the values extracted from the image.
                    extracted_kernel[ki + kernel_radius] = in[real_ki*width + j];
                }
                // The kernel has been fully extracted, time to execute kernel operation.
                out[current_pos] = kernel_operator(extracted_kernel);
            }
        }
    }

    template<typename IN_T, typename OUT_T, std::size_t K_SIZE>
    void hline_convolution(const Image<IN_T> &in, Image<OUT_T> &out, const std::function<OUT_T(std::array<IN_T, K_SIZE>&)> kernel_operator)
    {
        std::size_t current_pos;
        std::size_t height = in.get_height(), width = in.get_width();
        int kernel_radius;

        if(K_SIZE % 2 == 0) throw std::invalid_argument("Kernel is of invalid size.");

        kernel_radius = K_SIZE >> 1;

        std::array<IN_T, K_SIZE> extracted_kernel;

        for(std::size_t i = 0; i < height; ++i)
        {
            for(std::size_t j = 0; j < width; ++j)
            {
                current_pos = i * width + j;

                for(int kj = -kernel_radius; kj <= kernel_radius; ++kj)
                {
                    int real_kj = j + kj;

                    if(real_kj < 0) real_kj = 0;
                    else if (real_kj >= width) real_kj = width-1;

                    // Fill in the kernel that will be passed to the function with the values extracted from the image.
                    extracted_kernel[kj + kernel_radius] = in[i*width + real_kj];
                }
                // The kernel has been fully extracted, time to execute kernel operation.
                out[current_pos] = kernel_operator(extracted_kernel);
            }
        }
    }

} // namespace detail

template <typename IN_T, typename OUT_T>
void gaussian_blur_filter(const Image<IN_T> &in, Image<OUT_T> &out)
{
    std::size_t height = in.get_height(), width = in.get_width();
    Image<float> half_blurred(width, height, {});

    // An NxN gaussian blur can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
    // Using float as the intermediate pixel type to avoid detail loss between steps.

    detail::vline_convolution<IN_T, float, 5>(in, half_blurred, detail::kernel_op_gaussian_5_<IN_T, float>);
    detail::hline_convolution<float, OUT_T, 5>(half_blurred, out, detail::kernel_op_gaussian_5_<float, IN_T>);
}

template <typename IN_T, typename OUT_T>
void image_subtraction(const Image<IN_T> &in1, const Image<IN_T> &in2, Image<OUT_T> &out)
{
    std::size_t total = in1.get_total();

    for(std::size_t i = 0; i < total; ++i)
    {
        if(in1[i] > in2[i]) out[i] = in1[i] - in2[i];
        else                out[i] = in2[i] - in1[i];
    }
}

template <typename IN_T, typename OUT_T>
void double_threshold(const Image<IN_T> &in, Image<OUT_T> &out, const IN_T low_threshold, const IN_T high_threshold)
{
    std::size_t total = in.get_total();
    IN_T val;

    for(int i = 0; i < total; ++i)
    {
        val = in[i];
        if     (val >= high_threshold) out[i] = 1; // Strong
        else if(val < low_threshold)   out[i] = 0; // Culled
        else                           out[i] = 2; // Weak
    }
}

template <typename IN_T, typename OUT_T>
void single_threshold(const Image<IN_T> &in, Image<OUT_T> &out, const IN_T threshold)
{
    std::size_t total = in.get_total();
    IN_T val;

    for(int i = 0; i < total; ++i)
    {
        val = in[i];
        if     (val >= threshold) out[i] = 1; // Strong
        else                      out[i] = 0; // Culled
    }
}

template <typename IN_T, typename OUT_T>
void hysteresis(const Image<IN_T> &in, Image<OUT_T> &out)
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

        for(char ki = -1; ki < 2; ++ki)
        {
            for(char kj = -1; kj < 2; ++kj)
            {
                k_pos = current_pos + width*ki + kj;
                if(!visited_map[k_pos] && in[k_pos] == 2) strong_pixel_stack.push(k_pos);
                visited_map[k_pos] = true;
            }
        }
    }
}

template <typename IN_T, typename OUT_T>
void image_interpolation(const Image<IN_T> &from, const Image<IN_T> &to, Image<OUT_T> &out, const float ratio)
{
    std::size_t total = out.get_total();

    for(std::size_t i = 0; i < total; ++i)
    {
        out[i] = from[i] + ratio * (to[i] - from[i]); // Simplified from equation: from[i]*(1-ratio) + to[i]*ratio
    }
}


template <typename IN_T, typename OUT_T>
void dilation(const Image<IN_T> &in, Image<OUT_T> &out)
{
    std::size_t height = in.get_height(), width = in.get_width();
    Image<IN_T> half_dilated(width, height, {});

    detail::vline_convolution<IN_T, IN_T, 3>(in, half_dilated, detail::kernel_op_dilation_<IN_T, IN_T, 3>);
    detail::hline_convolution<IN_T, OUT_T, 3>(half_dilated, out, detail::kernel_op_dilation_<IN_T, OUT_T, 3>);
}


template<typename IN_T, typename OUT_T>
void downsample(const Image<IN_T> &in, Image<OUT_T> &out, std::size_t factor)
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
