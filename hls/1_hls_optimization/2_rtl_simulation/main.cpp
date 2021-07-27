#include <iostream>
#include <cstddef>
#include <cstring>
#include <vector>
#include <chrono>
#include <exception>

#include "motion_detector.hpp"
#include <opencv2/core.hpp>     // OpenCV is used to capture frames to feed into the motion detector
#include <opencv2/videoio.hpp>

int main(int argc, char** argv)
{
	/*
	uint16_t data_in[motdet::original_total] = {
		1000, 2000, 3000, 4000, 5010, 6000, 7000, 8000, 1000, 8000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1001, 2000, 3000, 4000, 5200, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1002, 2000, 3000, 4000, 5010, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1003, 2000, 3000, 4000, 5010, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1004, 2000, 3000, 4000, 5200, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1005, 2000, 3000, 4000, 5002, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1006, 2000, 3000, 4000, 5903, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1007, 2000, 3000, 4000, 5004, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1008, 2000, 3000, 4000, 5500, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1009, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1010, 2000, 3000, 4000, 5600, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1020, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1030, 2000, 3000, 4000, 5040, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1040, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1050, 2000, 3000, 4000, 5020, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 9000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1060, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1070, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1080, 2000, 3000, 4000, 5900, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1090, 2000, 3000, 4000, 5200, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1100, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1200, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1300, 2000, 3000, 4000, 5400, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1400, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1500, 2000, 3000, 4000, 5300, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1700, 2000, 3000, 4000, 8000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1800, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1900, 2000, 3000, 4000, 1000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1010, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1010, 2000, 3000, 4000, 5300, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1010, 2000, 3000, 4000, 2000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1010, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
		1020, 2000, 3000, 4000, 6000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
	};

	hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &image_in = *(new hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH>("in_main"));
	hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &conts_out = *(new hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH>("out_main"));

	for(uint32_t i = 0; i < motdet::original_total; i += motdet::motdet_reduction_factor)
	{
		motdet::Packed_pix packed;
		for(uint8_t k = 0; k < motdet::motdet_reduction_factor; ++k)
		{
			packed.pix[k] = data_in[i + k];
		}
		image_in.write(packed);
	}

	detect_motion(image_in, conts_out);

	motdet::Streamed_contour cont = conts_out.read();
	while(!cont.stream_end)
	{
		std::cout << "New contour" << std::endl;
	}
	std::cout << "No contours" << std::endl;
	*/

	// We will follow the same dynamic as in previous test main functions,
	// but here we wont record the result, just inform it is recording so that we know its working.

	std::string in_source = "C:/Users/bfant/Desktop/Work/University/TFG/motion_detector/data/in/test_motion.mp4";
    cv::VideoCapture cap;
	cap.open(in_source);

    if (!cap.isOpened()) throw std::runtime_error("Failed opening input.");

    // We will record for a few extra seconds after motion is no longer detected to avoid videos turning off and on intermittently.
    unsigned long long millis_keep_recording = 3000;
    unsigned long long millis_last_movement = 0;
    unsigned long long millis_prev_frame = 0, millis_frame = 0;
    unsigned int recordings_counter = 0; // We will create a separate video for each time movement is detected, use a counter to sequence videos.
    bool recording = false, first_iteration = true;

    // Start grabbing frames and checking for motion. Store the captured frame is a OpenCV Matrix (Mat).
    while(true)
    {
        // Create a container for the new frame that we are going to read, and fill it from the input source.
        cv::Mat frame;
        cap.read(frame);

        if(first_iteration)
        {
            millis_prev_frame = millis_frame = 0;
            first_iteration = false;
        }
        else
        {
            millis_prev_frame = millis_frame;
            millis_frame = cap.get(cv::CAP_PROP_POS_MSEC);
        }

        if(frame.empty() || millis_frame < millis_prev_frame) break; // Check for the end of the input video.

        if(millis_frame < 9750 || millis_frame > 10400) continue; // <<<<<< TESTING PURPOSES >>>>>>

        // Video did not end, get the data from the frame.

        unsigned char *rgb_data = (unsigned char *)frame.data;

        hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &image_in = *(new hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH>());
        hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &conts_out = *(new hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH>("out_main"));

        // Turn the RGB image to grayscale before sending to FPGA
        for(uint32_t i = 0; i < motdet::original_total; i += motdet::motdet_reduction_factor)
        {
        	motdet::Packed_pix packed;
        	for(uint8_t k = 0; k < motdet::motdet_reduction_factor; ++k)
        	{
        		uint32_t mapped = (i+k)*3;
        		packed.pix[k] = rgb_data[mapped] * 76.245 + rgb_data[mapped+1] * 149.685 + rgb_data[mapped+2] * 29.07;
        	}
        	image_in.write(packed);

        }

        detect_motion(image_in, conts_out); // Submit frame to FPGA for processing.


		motdet::Streamed_contour cont = conts_out.read();

        // Process the detected movement contours. Start or stop recording accordingly.

        if(recording) std::cout << "[REC] ";
        std::cout << "Got results for " << millis_frame << "." << std::endl;

        if(!cont.stream_end)
        {
            if(!recording)
            {
                // First movement detected in a while, start recording
                std::cout << "Motion detected. Recording..." << std::endl;
            }
            millis_last_movement = millis_frame;
            recording = true;

            while(!cont.stream_end){
            	cont = conts_out.read();
            	std::cout << "." << std::endl;
            }
        }
        else
        {
            if(recording && millis_last_movement + millis_keep_recording < millis_frame)
            {
                // No movement detected in feed after a while, closing recording.
                std::cout << "No motion detected. Closing recording " << recordings_counter << std::endl;
                recording = false;
            }
        }
    }

    return 0;
}
