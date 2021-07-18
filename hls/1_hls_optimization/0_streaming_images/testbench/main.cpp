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

        if(millis_frame < 10000 || millis_frame > 11000) continue; // <<<<<< TESTING PURPOSES >>>>>>

        // Video did not end, get the data from the frame.

        unsigned char *rgb_data = (unsigned char *)frame.data;

        hls::stream<uint16_t, MOTDET_STREAM_DEPTH> &image_in = *(new hls::stream<uint16_t, MOTDET_STREAM_DEPTH>());
        motdet::Contour_package conts;

        // Turn the RGB image to grayscale before sending to FPGA
        for(uint32_t i = 0; i < motdet::original_total; ++i)
        {
            uint32_t mapped = i*3;
            image_in.write(rgb_data[mapped] * 76.245 + rgb_data[mapped+1] * 149.685 + rgb_data[mapped+2] * 29.07);
        }

        detect_motion(image_in, conts); // Submit frame to FPGA for processing.

        // Process the detected movement contours. Start or stop recording accordingly.

        if(recording) std::cout << "[REC] ";
        std::cout << "Got results for " << millis_frame << "." << std::endl;

        if(conts.contour_count > 0)
        {
            if(!recording)
            {
                // First movement detected in a while, start recording
                std::cout << "Motion detected. Recording..." << std::endl;
            }
            millis_last_movement = millis_frame;
            recording = true;
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
