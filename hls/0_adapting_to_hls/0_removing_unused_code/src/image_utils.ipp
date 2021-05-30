
namespace detail
{
    template <typename IN_T, typename OUT_T>
    OUT_T kernel_op_median_3x3_(std::array<IN_T, 9> &p)
    {
        // This is the fastest known algorithm without making assumptions about the input.
        // "Borrowed" from http://ndevilla.free.fr/median/median.pdf

        pixel_sort_<IN_T>(p[1],p[2]); pixel_sort_<IN_T>(p[4],p[5]); pixel_sort_<IN_T>(p[7],p[8]);
        pixel_sort_<IN_T>(p[0],p[1]); pixel_sort_<IN_T>(p[3],p[4]); pixel_sort_<IN_T>(p[6],p[7]);
        pixel_sort_<IN_T>(p[1],p[2]); pixel_sort_<IN_T>(p[4],p[5]); pixel_sort_<IN_T>(p[7],p[8]);
        pixel_sort_<IN_T>(p[0],p[3]); pixel_sort_<IN_T>(p[5],p[8]); pixel_sort_<IN_T>(p[4],p[7]);
        pixel_sort_<IN_T>(p[3],p[6]); pixel_sort_<IN_T>(p[1],p[4]); pixel_sort_<IN_T>(p[2],p[5]);
        pixel_sort_<IN_T>(p[4],p[7]); pixel_sort_<IN_T>(p[4],p[2]); pixel_sort_<IN_T>(p[6],p[4]);
        pixel_sort_<IN_T>(p[4],p[2]);

        return(p[4]);
    }

    template <typename IN_T, typename OUT_T>
    OUT_T kernel_op_gaussian_5_(const std::array<IN_T, 5> &p)
    {
        double total = 0;
        for(std::size_t i = 0; i < 5; ++i) total += p[i] * gaussian_kernel_5_[i];
        return total;
    }

    template <typename IN_T, typename OUT_T>
    OUT_T kernel_op_sobel_h_3x3_(const std::array<IN_T, 9> &p)
    {
        OUT_T total = 0;
        for(std::size_t i = 0; i < 9; ++i) total += p[i] * sobel_h_kernel_3x3_[i];
        return total;
    }

    template <typename IN_T, typename OUT_T>
    OUT_T kernel_op_sobel_v_3x3_(const std::array<IN_T, 9> &p)
    {
        OUT_T total = 0;
        for(std::size_t i = 0; i < 9; ++i) total += p[i] * sobel_v_kernel_3x3_[i];
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

    template <typename IN_T, typename OUT_T, std::size_t SIZE_K>
    OUT_T kernel_op_erosion_(const std::array<IN_T, SIZE_K> &p)
    {
        for(const IN_T &i: p)
        {
            if (i <= 0) return 0;
        }
        return 1;
    }


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

    template<typename IN_T, typename OUT_T, std::size_t K_SIZE>
    void square_convolution(const Image<IN_T> &in, Image<OUT_T> &out, const std::function<OUT_T(std::array<IN_T, K_SIZE>&)> kernel_operator)
    {
        std::size_t current_pos;
        std::size_t height = in.get_height(), width = in.get_width();
        int kernel_side, kernel_radius;

        kernel_side = detail::fast_sqrt_(K_SIZE); // Since a kernel is strictly square, the square root is the side length.
        if(kernel_side == 0 || kernel_side % 2 == 0) throw std::invalid_argument("Kernel is of invalid size.");

        kernel_radius = kernel_side >> 1;

        std::array<IN_T, K_SIZE> extracted_kernel;

        for(std::size_t i = 0; i < height; ++i)
        {
            for(std::size_t j = 0; j < width; ++j)
            {
                current_pos = i * width + j;

                for(int ki = -kernel_radius; ki <= kernel_radius; ++ki)
                {
                    for(int kj = -kernel_radius; kj <= kernel_radius; ++kj)
                    {
                        int real_ki = i + ki;
                        int real_kj = j + kj;

                        if(real_ki < 0) real_ki = 0;
                        else if (real_ki >= height) real_ki = height-1;

                        if(real_kj < 0) real_kj = 0;
                        else if (real_kj >= width) real_kj = width-1;

                        // Fill in the kernel that will be passed to the function with the values extracted from the image.
                        extracted_kernel[(ki + kernel_radius)*kernel_side + kj + kernel_radius] = in[real_ki*width + real_kj];
                    }
                }
                // The kernel has been fully extracted, time to execute kernel operation.
                out[current_pos] = kernel_operator(extracted_kernel);
            }
        }
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
void median_filter(const Image<IN_T> &in, Image<OUT_T> &out)
{
    detail::square_convolution<IN_T, OUT_T, 9>(in, out, detail::kernel_op_median_3x3_<IN_T, OUT_T>);
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

template <typename OUT_T>
void canny_edge_detection_8b(const Image<unsigned char> &in, Image<OUT_T> &out, const unsigned char low_threshold, const unsigned char high_threshold)
{
    std::size_t height = in.get_height(), width = in.get_width();

    Image<unsigned char> gra_image(width, height, {}), thresh_image(width, height, {}), mag_image(width, height, {}), sup_image(width, height, {});

    sobel_edge_detection_8b<unsigned char>           (in, mag_image, gra_image                                                            );
    non_max_suppression<unsigned char, unsigned char>(    mag_image, gra_image, sup_image                                                 );
    double_threshold<unsigned char, unsigned char>   (                          sup_image, thresh_image,     low_threshold, high_threshold);
    hysteresis<unsigned char, OUT_T>                 (                                     thresh_image, out                              );
}

template<typename OUT_T>
void sobel_edge_detection_8b(const Image<unsigned char> &in, Image<OUT_T> &out_magnitude, Image<unsigned char> &out_gradient)
{
    short aux_mag; // input is 8b and we need a signed type of 10b. Closest candidate is short.
    double aux_gra;

    std::size_t height = in.get_height(), width = in.get_width(), total = in.get_total();
    Image<short> sobel_h(width, height, {}), sobel_v(width, height, {});

    detail::square_convolution<unsigned char, short, 9>(in, sobel_h, detail::kernel_op_sobel_h_3x3_<unsigned char, short>);
    detail::square_convolution<unsigned char, short, 9>(in, sobel_v, detail::kernel_op_sobel_v_3x3_<unsigned char, short>);

    for(std::size_t i = 0; i < total; ++i)
    {
        aux_mag = detail::fast_sqrt_(sobel_h[i] * sobel_h[i] + sobel_v[i] * sobel_v[i]);

        // Intensity of the edge from 0 (B) to 255 (W)
        out_magnitude[i] = aux_mag > 255 ? 255 : aux_mag;

        // Angle of the edge in radians. Returns the angle of radians from origin (0, 0) to point (sobel_h[i], sobel_v[i])
        aux_gra = atan2(sobel_h[i], sobel_v[i]);

        // Convert rad to deg: degrees = radians * 180 * PI
        // To map from 360 deg range to 255 deg, multiply by 0.71: degrees = radians * 127.8 * PI
        // Fuse that number with PI and the final instruction is: degrees = radians * 401.45
        out_gradient[i] = aux_gra * 401.45;
    }
}

template<typename IN_T, typename OUT_T>
void non_max_suppression(const Image<IN_T> &in_magnitude, const Image<unsigned char> &in_gradient, Image<OUT_T> &out)
{
    IN_T q, r;
    bool valid_pixel;
    unsigned short deg;
    std::size_t height = in_magnitude.get_height(), width = in_magnitude.get_width(), current_pos;

    for(std::size_t i = 0; i < height; ++i)
    {
        for(std::size_t j = 0; j < width; ++j)
        {
            current_pos = i * width + j;

            deg = in_gradient[current_pos] * 1.4; // Map the degrees from 255 back to 360. This is an approximation, but it's good enough.
            valid_pixel = false;

            if((0 <= deg < 22.5) || (157.5 <= deg <= 180)){ // Angle 0 (-)
                if(!(j == 0 || j == width-1)){
                    q = in_magnitude[current_pos+1];
                    r = in_magnitude[current_pos-1];
                    valid_pixel = true;
                }
            }
            else if(22.5 <= deg < 67.5){                    // Angle 45 (/)
                if(!(j == 0 || j == width-1 || i == 0 || i == height-1)){
                    q = in_magnitude[current_pos+width-1];
                    r = in_magnitude[current_pos-width+1];
                    valid_pixel = true;
                }
            }
            else if(67.5 <= deg < 112.5){                   // Angle 90 (|)
                if(!(i == 0 || i == height-1)){
                    q = in_magnitude[current_pos+width];
                    r = in_magnitude[current_pos-width];
                    valid_pixel = true;
                }
            }
            else if(112.5 <= deg < 157.5){                  // Angle 135 (\)
                if(!(j == 0 || j == width-1 || i == 0 || i == height-1)){
                    q = in_magnitude[current_pos-width-1];
                    r = in_magnitude[current_pos+width+1];
                    valid_pixel = true;
                }
            }

            if(valid_pixel && in_magnitude[current_pos] >= q && in_magnitude[current_pos] >= r) out[current_pos] = in_magnitude[current_pos];
            else out[current_pos] = 0;
        }
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

template <typename IN_T, typename OUT_T>
void erosion(const Image<IN_T> &in, Image<OUT_T> &out)
{
    std::size_t height = in.get_height(), width = in.get_width();
    Image<IN_T> half_eroded(width, height, {});

    detail::vline_convolution<IN_T, IN_T, 3>(in, half_eroded, detail::kernel_op_erosion_<IN_T, IN_T, 3>);
    detail::hline_convolution<IN_T, OUT_T, 3>(half_eroded, out, detail::kernel_op_erosion_<IN_T, OUT_T, 3>);
}

template<typename IN_T, typename OUT_T>
void reescale_pix_length(const Image<IN_T> &in, Image<OUT_T> &out, IN_T in_max_val, OUT_T out_max_val)
{
    std::size_t total = out.get_total();

    double slope = (double)out_max_val/(double)in_max_val;

    for(std::size_t i = 0; i < total; ++i)
    { out[i] = ((float)in[i])*slope; }
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


template<typename IN_T, typename OUT_T>
void upsample(const Image<IN_T> &in, Image<OUT_T> &out, std::size_t factor)
{
    std::size_t in_height = in.get_height(), in_width = in.get_width();
    std::size_t out_height = out.get_height(), out_width = out.get_width();

    for(std::size_t i = 0; i < in_height; ++i)
    {
        std::size_t scaled_i = i*factor;
        for(std::size_t j = 0; j < in_width; ++j)
        {
            std::size_t scaled_pos = scaled_i*out_width + j*factor;
            IN_T val = in[i*in_width + j];
            for(std::size_t box_i = 0; box_i < factor; ++box_i)
            {
                std::size_t box_i_dis = box_i*out_width;
                for(std::size_t box_j = 0; box_j < factor; ++box_j)
                {
                    out[scaled_pos + box_i_dis + box_j] = val;
                }
            }
        }
    }
}
