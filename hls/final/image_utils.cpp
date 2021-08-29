#include "image_utils.hpp"

#include <iostream>

namespace motdet
{
    namespace imgutil
    {
        namespace // Anonymous namespace
        {
        	ap_uint<16> motdet_reference[MOTDET_HEIGHT][MOTDET_WIDTH];
            ap_uint<7> gaussian_kernel[5] = { 16, 62, 99, 62, 16 }; // Total 255

            bool has_reference = false;
        } // Anonymous namespace

        void gaussian_blur_filter_vline(hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &out)
		{
		#ifndef __SYNTHESIS__
        	ap_uint<16>** buffer = new ap_uint<16>*[5];
			for(ap_uint<3> i = 0; i < 5; ++i) buffer[i] = new ap_uint<16>[MOTDET_WIDTH];
		#else
			ap_uint<16> buffer[5][MOTDET_WIDTH];
#pragma HLS ARRAY_PARTITION variable=buffer dim=1 complete
		#endif

			ap_uint<3> buffer_ptr = 0;

			// The first row we read will need to fill out the 2 extra pixels outside the border of the image.
			for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j)
			{
#pragma HLS PIPELINE
				ap_uint<16> curr_val = in.read();
				buffer[3][j] = curr_val;
				buffer[4][j] = curr_val;
				buffer[0][j] = curr_val;
			}

			// The second row just needs to be written to the buffer, but we cannot still output results because we only have 4 rows out of 5.
			for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j){
#pragma HLS PIPELINE
				buffer[1][j] = in.read();
			}

			// Now iterate over the rest of the rows, now each row we get, we can output results.
			for(ap_uint<9> i = 2; i < MOTDET_HEIGHT; ++i)
			{
				buffer_ptr = i%5;
				for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j)
				{
#pragma HLS PIPELINE
					buffer[buffer_ptr][j] = in.read();

					ap_uint<24> res = 0;
					for(ap_uint<3> k = 0; k < 5; ++k) res += buffer[(buffer_ptr+k+1)%5][j] * gaussian_kernel[k];
					out.write(res/255);
				}
			}

			// Now we have iterated the whole image, but we only have outputted MOTDET_HEIGHT-2 rows.
			// Output those last 2 rows now extending the pixels at the border of the image.
			for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j){
#pragma HLS PIPELINE
				buffer[buffer_ptr][j] = buffer[(buffer_ptr-1)%5][j];

				ap_uint<24> res = 0;
				for(ap_uint<3> k = 0; k < 5; ++k) res += buffer[(buffer_ptr+k+1)%5][j] * gaussian_kernel[k];
				out.write(res/255);
			}

			for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j){
#pragma HLS PIPELINE
				buffer[(buffer_ptr+1)%5][j] = buffer[buffer_ptr][j];

				ap_uint<24> res = 0;
				for(ap_uint<3> k = 0; k < 5; ++k) res += buffer[(buffer_ptr+k+2)%5][j] * gaussian_kernel[k];
				out.write(res/255);
			}

		#ifndef __SYNTHESIS__
			for(ap_uint<3> i = 0; i < 5; ++i) delete[] buffer[i];
			delete[] buffer;
		#endif
		}

        void gaussian_blur_filter_hline(hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &out)
        {
        	ap_uint<16> buffer[5];
            ap_uint<24> res = 0;

            for(ap_uint<9> i = 0; i < MOTDET_HEIGHT; ++i)
            {
            	ap_uint<16> init_val = in.read();

                buffer[3] = init_val;
                buffer[4] = init_val;
                buffer[0] = init_val;
                buffer[1] = in.read();
                buffer[2] = in.read();

                for(ap_uint<9> j = 3; j < MOTDET_WIDTH; ++j)
                {
#pragma HLS PIPELINE
                	res = 0;
					for(ap_uint<3> k = 0; k < 5; ++k) res += buffer[(j+k)%5] * gaussian_kernel[k];
					out.write(res/255);

					buffer[j%5] = in.read();
                }

                res = 0;
                for(ap_uint<3> k = 0; k < 5; ++k)
                {
#pragma HLS PIPELINE
                	res += buffer[(MOTDET_WIDTH+k)%5] * gaussian_kernel[k];
                }
				out.write(res/255);

                buffer[MOTDET_WIDTH%5] = buffer[(MOTDET_WIDTH-1)%5];
                res = 0;
                for(ap_uint<3> k = 0; k < 5; ++k)
                {
#pragma HLS PIPELINE
                	res += buffer[(MOTDET_WIDTH+k+1)%5] * gaussian_kernel[k];
                }
                out.write(res/255);

                buffer[(MOTDET_WIDTH+1)%5] = buffer[MOTDET_WIDTH%5];
                res = 0;
                for(ap_uint<3> k = 0; k < 5; ++k)
                {
#pragma HLS PIPELINE
                	res += buffer[(MOTDET_WIDTH+k+2)%5] * gaussian_kernel[k];
                }
                out.write(res/255);
            }
        }

        void gaussian_blur(hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &out)
        {

        	#pragma HLS DATAFLOW
            hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> half_blurred("half-blurred");

            // An NxN gaussian blur can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
            gaussian_blur_filter_vline(in, half_blurred);
            gaussian_blur_filter_hline(half_blurred, out);
        }

        void downsample(hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &out)
		{
		#ifndef __SYNTHESIS__
        	ap_uint<20> *buffer = new ap_uint<20>[MOTDET_WIDTH];
		#else
			ap_uint<20> buffer[MOTDET_WIDTH];
		#endif

			ap_uint<5> squared_red_factor = MOTDET_REDUCTION_FACTOR*MOTDET_REDUCTION_FACTOR;

			for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j) buffer[j] = 0;

			for(ap_uint<11> i = 0; i < ORIGINAL_HEIGHT; ++i)
			{
				// Keep filling the buffer
				for(ap_uint<9> motdet_j = 0; motdet_j < MOTDET_WIDTH; ++motdet_j)
				{
#pragma HLS PIPELINE
					motdet::Packed_pix packed = in.read();
					ap_uint<18> total = 0;
					for(ap_uint<3> k = 0; k < MOTDET_REDUCTION_FACTOR; ++k) total += packed.pix[k];
					buffer[motdet_j] += total;
				}

				// Processed the last line into the buffer, output.
				if(!((i+1) % MOTDET_REDUCTION_FACTOR))
				{
					// Line that completes the buffer, start outputting
					for(ap_uint<9> motdet_j = 0; motdet_j < MOTDET_WIDTH; ++motdet_j)
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

        void apply_reference(hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &out)
        {
        	float update_ratio = has_reference ? motdet_frame_update_ratio : 1.0;
			has_reference = true;

			for(ap_uint<9> i = 0; i < MOTDET_HEIGHT; ++i)
			{
				for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j)
				{
#pragma HLS PIPELINE
					ap_uint<16> from_val = motdet_reference[i][j];
					ap_uint<16> to_val = in.read();
					ap_uint<16> new_val = from_val + update_ratio * (to_val - from_val); // Simplified from equation: from*(1-ratio) + to*ratio
					motdet_reference[i][j] = new_val;

					out.write(hls::abs(to_val - new_val));
				}
			}
        }

        void single_threshold(hls::stream<ap_uint<16>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &out)
        {
            for(ap_uint<17> i = 0; i < MOTDET_TOTAL; ++i){
#pragma HLS PIPELINE
            	out.write(in.read() > motdet_threshold ? 1 : 0);
            }
        }

        void dilation_vline(hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &out)
        {
        #ifndef __SYNTHESIS__
        	ap_uint<1>** buffer = new ap_uint<1>*[2];
            for(ap_uint<2> i = 0; i < 2; ++i) buffer[i] = new ap_uint<1>[MOTDET_WIDTH];
        #else
            ap_uint<1> buffer[2][MOTDET_WIDTH];
        #endif
            ap_uint<3> buffer_ptr;

            // The first row we read will need to fill out the pixels outside the border of the image as if they were 0.
            for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j){
#pragma HLS PIPELINE
            	buffer[1][j] = 0;
            }

            // The second row just needs to be written to the buffer, but we cannot still output results because we only have 2 rows out of 3.
            for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j)
            {
#pragma HLS PIPELINE
            	buffer[0][j] = in.read();
            }

            // Now iterate over the rest of the rows, now each row we get, we can output results.
            for(ap_uint<9> i = 1; i < MOTDET_HEIGHT; ++i)
            {
                buffer_ptr = i%2;
                for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j)
                {
#pragma HLS PIPELINE
                	ap_uint<1> val_read = in.read();

                    if(val_read || buffer[0][j] || buffer[1][j]) out.write(1);
                    else out.write(0);

                    buffer[buffer_ptr][j] = val_read;
                }
            }

            // Now we have iterated the whole image, but we only have outputted MOTDET_HEIGHT-1 rows.
            // Output those last row extending the image with zeros at the bottom.
            for(ap_uint<9> j = 0; j < MOTDET_WIDTH; ++j){
#pragma HLS PIPELINE
                if(buffer[0][j] || buffer[1][j]) out.write(1);
                else out.write(0);
            }

        #ifndef __SYNTHESIS__
            for(ap_uint<2> i = 0; i < 2; ++i) delete[] buffer[i];
            delete[] buffer;
        #endif
        }

        void dilation_hline(hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &out)
        {
        	ap_uint<1> buffer[2];

            for(ap_uint<9> i = 0; i < MOTDET_HEIGHT; ++i)
            {
                buffer[0] = 0;
                buffer[1] = in.read();

                for(ap_uint<9> j = 1; j < MOTDET_WIDTH; ++j)
                {
#pragma HLS PIPELINE
                	ap_uint<1> val_read = in.read();

                    if(val_read || buffer[0] || buffer[1]) out.write(1);
                    else out.write(0);

                    buffer[j%2] = val_read;
                }

                if(buffer[0] || buffer[1]) out.write(1);
                else out.write(0);
            }
        }

        void dilation(hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &in, hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> &out)
        {
#pragma HLS DATAFLOW
            hls::stream<ap_uint<1>, MOTDET_STREAM_DEPTH> half_dilated;

            // An NxN dilation can be decomposed into 2 1-dimensional kernels, N vertical and N horizontal.
            dilation_vline(in, half_dilated);
            dilation_hline(half_dilated, out);
        }

    } // namespace imgutil
} // namespace motdet
