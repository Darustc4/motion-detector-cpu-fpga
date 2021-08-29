#include "image_utils.hpp"

#include <iostream>

namespace motdet
{
    namespace imgutil
    {
        namespace // Anonymous namespace
        {
            uint16_t motdet_reference[motdet_height][motdet_width];
            uint8_t gaussian_kernel[5] = { 16, 62, 99, 62, 16 }; // Total 255

            bool has_reference = false;
        } // Anonymous namespace

        void gaussian_blur_filter_vline(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out)
		{
		#ifndef __SYNTHESIS__
			uint16_t** buffer = new uint16_t*[5];
			for(uint8_t i = 0; i < 5; ++i) buffer[i] = new uint16_t[motdet_width];
		#else
			uint16_t buffer[5][motdet_width];
#pragma HLS ARRAY_PARTITION variable=buffer dim=1 complete
		#endif
			uint8_t buffer_ptr = 0;

			// The first row we read will need to fill out the 2 extra pixels outside the border of the image.
			for(uint16_t j = 0; j < motdet_width; ++j)
			{
#pragma HLS PIPELINE
				uint16_t curr_val = in.read();
				buffer[3][j] = curr_val;
				buffer[4][j] = curr_val;
				buffer[0][j] = curr_val;
			}

			// The second row just needs to be written to the buffer, but we cannot still output results because we only have 4 rows out of 5.
			for(uint16_t j = 0; j < motdet_width; ++j){
#pragma HLS PIPELINE
				buffer[1][j] = in.read();
			}

			// Now iterate over the rest of the rows, now each row we get, we can output results.
			for(uint16_t i = 2; i < motdet_height; ++i)
			{
				buffer_ptr = i%5;
				for(uint16_t j = 0; j < motdet_width; ++j)
				{
#pragma HLS PIPELINE
					buffer[buffer_ptr][j] = in.read();

					uint32_t res = 0;
					for(uint8_t k = 0; k < 5; ++k) res += buffer[(buffer_ptr+k+1)%5][j] * gaussian_kernel[k];
					out.write(res/255);
				}
			}

			// Now we have iterated the whole image, but we only have outputted motdet_height-2 rows.
			// Output those last 2 rows now extending the pixels at the border of the image.
			for(uint16_t j = 0; j < motdet_width; ++j){
#pragma HLS PIPELINE
				buffer[buffer_ptr][j] = buffer[(buffer_ptr-1)%5][j];

				uint32_t res = 0;
				for(uint8_t k = 0; k < 5; ++k) res += buffer[(buffer_ptr+k+1)%5][j] * gaussian_kernel[k];
				out.write(res/255);
			}

			for(uint16_t j = 0; j < motdet_width; ++j){
#pragma HLS PIPELINE
				buffer[(buffer_ptr+1)%5][j] = buffer[buffer_ptr][j];

				uint32_t res = 0;
				for(uint8_t k = 0; k < 5; ++k) res += buffer[(buffer_ptr+k+2)%5][j] * gaussian_kernel[k];
				out.write(res/255);
			}

		#ifndef __SYNTHESIS__
			for(uint8_t i = 0; i < 5; ++i) delete buffer[i];
			delete[] buffer;
		#endif
		}

        void gaussian_blur_filter_hline(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out)
        {
            uint16_t buffer[5];
            uint32_t res = 0;

            for(uint16_t i = 0; i < motdet_height; ++i)
            {
                uint16_t init_val = in.read();

                buffer[3] = init_val;
                buffer[4] = init_val;
                buffer[0] = init_val;
                buffer[1] = in.read();
                buffer[2] = in.read();

                for(uint16_t j = 3; j < motdet_width; ++j)
                {
#pragma HLS PIPELINE
                	res = 0;
					for(uint8_t k = 0; k < 5; ++k) res += buffer[(j+k)%5] * gaussian_kernel[k];
					out.write(res/255);

					buffer[j%5] = in.read();
                }

                res = 0;
                for(uint8_t k = 0; k < 5; ++k)
                {
#pragma HLS PIPELINE
                	res += buffer[(motdet_width+k)%5] * gaussian_kernel[k];
                }
				out.write(res/255);

                buffer[motdet_width%5] = buffer[(motdet_width-1)%5];
                res = 0;
                for(uint8_t k = 0; k < 5; ++k)
                {
#pragma HLS PIPELINE
                	res += buffer[(motdet_width+k+1)%5] * gaussian_kernel[k];
                }
                out.write(res/255);

                buffer[(motdet_width+1)%5] = buffer[motdet_width%5];
                res = 0;
                for(uint8_t k = 0; k < 5; ++k)
                {
#pragma HLS PIPELINE
                	res += buffer[(motdet_width+k+2)%5] * gaussian_kernel[k];
                }
                out.write(res/255);
            }
        }

        void gaussian_blur(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out)
        {

        	#pragma HLS DATAFLOW
            hls::stream<uint16_t, MOTDET_STREAM_DEPTH> half_blurred("half-blurred");

            // An NxN gaussian blur can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
            gaussian_blur_filter_vline(in, half_blurred);
            gaussian_blur_filter_hline(half_blurred, out);
        }

        void downsample(hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out)
		{
		#ifndef __SYNTHESIS__
		   uint32_t *buffer = new uint32_t[motdet_width];
		#else
		   uint32_t buffer[motdet_width];
		#endif

		   uint8_t squared_red_factor = motdet_reduction_factor*motdet_reduction_factor;

		   for(uint16_t j = 0; j < motdet_width; ++j) buffer[j] = 0;

		   for(uint16_t i = 0; i < original_height; ++i)
		   {
			   // Keep filling the buffer
			   for(uint16_t motdet_j = 0; motdet_j < motdet_width; ++motdet_j)
				   {
#pragma HLS PIPELINE
				   motdet::Packed_pix packed = in.read();
				   uint32_t total = 0;
				   for(uint8_t k = 0; k < motdet_reduction_factor; ++k) total += packed.pix[k];
				   buffer[motdet_j] += total;
			   }

			   // Processed the last line into the buffer, output.
			   if(!((i+1) % motdet_reduction_factor))
			   {
				   // Line that completes the buffer, start outputting
				   for(uint16_t motdet_j = 0; motdet_j < motdet_width; ++motdet_j)
				   {
#pragma HLS PIPELINE
					   out.write(buffer[motdet_j]/squared_red_factor);
					   buffer[motdet_j] = 0;
				   }
			   }
		   }

		#ifndef __SYNTHESIS__
		   delete[] buffer;
		#endif
		}

        void t_pipe_interpolate_reference(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &t_piped_out)
        {
        	float update_ratio = has_reference ? motdet_frame_update_ratio : 1.0;
        	has_reference = true;

            for(uint16_t i = 0; i < motdet_height; ++i)
            {
                for(uint16_t j = 0; j < motdet_width; ++j)
                {
#pragma HLS PIPELINE
                    uint16_t from_val = motdet_reference[i][j];
                    uint16_t to_val = in.read();
                    t_piped_out.write(to_val);

                    motdet_reference[i][j] = from_val + update_ratio * (to_val - from_val); // Simplified from equation: from*(1-ratio) + to*ratio
                }
            }
        }

        void reference_subtraction(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &out)
        {
            for(uint16_t i = 0; i < motdet_height; ++i)
            {
                for(uint16_t j = 0; j < motdet_width; ++j)
                {
#pragma HLS PIPELINE
                    out.write(hls::abs(in.read() - motdet_reference[i][j]));
                }
            }
        }

        void single_threshold(hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &out)
        {
            for(uint32_t i = 0; i < motdet_total; ++i){
#pragma HLS PIPELINE
            	out.write(in.read() > motdet_threshold ? 1 : 0);
            }
        }

        void dilation_vline(hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &out)
        {
        #ifndef __SYNTHESIS__
            uint8_t** buffer = new uint8_t*[2];
            for(uint8_t i = 0; i < 2; ++i) buffer[i] = new uint8_t[motdet_width];
        #else
            uint8_t buffer[2][motdet_width];
        #endif
            uint8_t buffer_ptr;

            // The first row we read will need to fill out the pixels outside the border of the image as if they were 0.
            for(uint16_t j = 0; j < motdet_width; ++j){
#pragma HLS PIPELINE
            	buffer[1][j] = 0;
            }

            // The second row just needs to be written to the buffer, but we cannot still output results because we only have 2 rows out of 3.
            for(uint16_t j = 0; j < motdet_width; ++j)
            {
#pragma HLS PIPELINE
            	buffer[0][j] = in.read();
            }

            // Now iterate over the rest of the rows, now each row we get, we can output results.
            for(uint16_t i = 1; i < motdet_height; ++i)
            {
                buffer_ptr = i%2;
                for(uint16_t j = 0; j < motdet_width; ++j)
                {
#pragma HLS PIPELINE
                    uint8_t val_read = in.read();

                    if(val_read || buffer[0][j] || buffer[1][j]) out.write(1);
                    else out.write(0);

                    buffer[buffer_ptr][j] = val_read;
                }
            }

            // Now we have iterated the whole image, but we only have outputted motdet_height-1 rows.
            // Output those last row extending the image with zeros at the bottom.
            for(uint16_t j = 0; j < motdet_width; ++j){
#pragma HLS PIPELINE
                if(buffer[0][j] || buffer[1][j]) out.write(1);
                else out.write(0);
            }

        #ifndef __SYNTHESIS__
            for(uint8_t i = 0; i < 2; ++i) delete buffer[i];
            delete[] buffer;
        #endif
        }

        void dilation_hline(hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &out)
        {
            uint8_t buffer[2];

            for(uint16_t i = 0; i < motdet_height; ++i)
            {
                buffer[0] = 0;
                buffer[1] = in.read();

                for(uint16_t j = 1; j < motdet_width; ++j)
                {
#pragma HLS PIPELINE
                    uint8_t val_read = in.read();

                    if(val_read || buffer[0] || buffer[1]) out.write(1);
                    else out.write(0);

                    buffer[j%2] = val_read;
                }

                if(buffer[0] || buffer[1]) out.write(1);
                else out.write(0);
            }
        }

        void dilation(hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &in, hls::stream<uint8_t, MOTDET_STREAM_DEPTH> &out)
        {
#pragma HLS DATAFLOW
            hls::stream<uint8_t, MOTDET_STREAM_DEPTH> half_dilated;

            // An NxN dilation can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
            dilation_vline(in, half_dilated);
            dilation_hline(half_dilated, out);
        }

    } // namespace imgutil
} // namespace motdet
