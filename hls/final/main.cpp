#include <iostream>
#include <cstddef>
#include <cstring>
#include <vector>
#include <chrono>
#include <exception>

#include "motion_detector.hpp"
#include <opencv2/core.hpp>     // OpenCV is used to capture frames to feed into the motion detector
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

int main(int argc, char** argv)
{
	// We will follow the same dynamic as in previous test main functions,
	// but here we wont record the result, just inform it is recording so that we know its working.

	std::string in_source = "C:/Users/bfant/Desktop/Work/University/TFG/motion_detector/data/in/test_motion.mp4";
	std::string out_recordings_dir = "C:/Users/bfant/Desktop/Work/University/TFG/motion_detector/data/out/";
    cv::VideoCapture cap;
	cap.open(in_source);

    if (!cap.isOpened()) throw std::runtime_error("Failed opening input.");

    // We will record for a few extra seconds after motion is no longer detected to avoid videos turning off and on intermittently.
    unsigned long long millis_keep_recording = 3000;
    unsigned long long millis_last_movement = 0;
    unsigned long long millis_prev_frame = 0, millis_frame = 0;
    unsigned int recordings_counter = 0; // We will create a separate video for each time movement is detected, use a counter to sequence videos.
    bool recording = false, first_iteration = true;

    cv::VideoWriter writer;
	int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v'); // Saving to .mp4
	unsigned char fps = cap.get(cv::CAP_PROP_FPS);
	std::size_t width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	std::size_t height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	cv::Size stream_res = cv::Size(width, height);

	std::vector<motdet::Contour> detected_conts;

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

        if(millis_frame < 9500 || millis_frame > 20000) continue;

        // Video did not end, get the data from the frame.

        unsigned char *rgb_data = (unsigned char *)frame.data;

        hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH> &image_in = *(new hls::stream<motdet::Packed_pix, MOTDET_STREAM_DEPTH>());
        hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH> &conts_out = *(new hls::stream<motdet::Streamed_contour, MOTDET_STREAM_DEPTH>("out_main"));

        // Turn the RGB image to grayscale before sending to FPGA
        for(uint32_t i = 0; i < ORIGINAL_TOTAL; i += MOTDET_REDUCTION_FACTOR)
        {
        	motdet::Packed_pix packed;
        	for(uint8_t k = 0; k < MOTDET_REDUCTION_FACTOR; ++k)
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
            	std::string file = "motion_" + std::to_string(recordings_counter) + ".mp4";
            	std::string rec_path = out_recordings_dir + file;
				writer.open(rec_path, codec, fps, stream_res, true);

                std::cout << "Motion detected. Recording..." << std::endl;
            }
            millis_last_movement = millis_frame;
            recording = true;

            detected_conts.clear();
            while(!cont.stream_end){
            	detected_conts.push_back(std::move(cont.contour));
            	cont = conts_out.read();
            }
        }
        else
        {
            if(recording && millis_last_movement + millis_keep_recording < millis_frame)
            {
                // No movement detected in feed after a while, closing recording.
                std::cout << "No motion detected. Closing recording " << recordings_counter << std::endl;

                ++recordings_counter;
				recording = false;
				writer.release();
            }
        }

        if(recording)
		{
			// Draw the detected motion onto the recovered frame as rectangles.
			for(motdet::Contour &cont : detected_conts)
			{
				// Draw the detected movement on screen

				cv::Point pt1(cont.bb_tl_x, cont.bb_tl_y);
				cv::Point pt2(cont.bb_br_x, cont.bb_br_y);
				cv::rectangle(frame, pt1, pt2, cv::Scalar(0, 255, 0));
			}

			// Save the edited frame into the video we are recording.
			writer.write(frame);
		}
    }

    return 0;
}
