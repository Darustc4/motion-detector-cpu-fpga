#include <iostream>
#include <cstddef>
#include <filesystem>
#include <cstring>
#include <vector>
#include <chrono>
#include <exception>

#include <motion_detector.hpp>
#include <opencv2/core.hpp>     // OpenCV is used to capture frames to feed into the motion detector
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

namespace fs = std::filesystem; // Rename namespaces for convenience.
namespace md = motdet;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cerr <<
        "ERROR: Missing parameters" << std::endl <<
        "Specify the following parameters: " << std::endl <<
        " - input: Either 'camera' which will capture from connected USB cam or a path to a .mp4" << std::endl <<
        " - output: Folder to store captured video in. Will create .mp4 files with date when motion is detected. NOTE: All files are deleted in the directory." << std::endl;

        return 0;
    }

    std::string in_source;
    std::string out_dir;

    // Get input source
    in_source = argv[1];
    if(in_source != "camera")
    {
        if(!fs::exists(in_source)) throw std::invalid_argument("The selected input video file is not accessible or does not exist.");
    }

    // Get output directory
    out_dir = argv[2];
    fs::path out_recordings_dir(out_dir);
    if(!fs::exists(out_recordings_dir)) throw std::invalid_argument("The selected output directory is not accessible or does not exist.");
    fs::remove_all(out_recordings_dir / "*");   // Clear the output directory.

    // At this point, all input parameters have been validated and stored.

    cv::VideoCapture cap;

    // Open either a connected camera or an input video.
    if(in_source == "camera")
    {
        int device_id = 0;             // 0 = open default camera
        int api_id = cv::CAP_ANY;      // 0 = autodetect default API
        cap.open(device_id, api_id);
        std::cout << "Using camera input..." << std::endl;
    }
    else
    {
        cap.open(in_source);
        std::cout << "Using .mp4 input..." << std::endl;
    }
    if (!cap.isOpened()) throw std::runtime_error("Failed opening input.");

    // When motion is detected a new .mp4 video will eb created in the output directory.
    // In order to do this, create a .mp4 writer from OpenCV.

    cv::VideoWriter writer;
    int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v'); // Saving to .mp4
    unsigned char fps = cap.get(cv::CAP_PROP_FPS);
    std::size_t width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    std::size_t height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size stream_res = cv::Size(width, height);

    // Create the Motion detector object from the library.
    md::Motion_detector motion_detector(0.0067);

    // We will record for a few extra seconds after motion is no longer detected to avoid videos turnign off and on intermittently.
    unsigned long long millis_keep_recording = 3000;
    unsigned long long millis_last_movement = 0;
    unsigned long long millis_prev_frame = 0, millis_frame = 0;
    unsigned int recordings_counter = 0; // We will create a separate video for each time movement is detected, use a counter to sequence videos.
    bool recording = false, first_iteration = true;

    auto t_video0 = high_resolution_clock::now();

    // Start grabbing frames and checking for motion. Store the captured frame is a OpenCV Matrix (Mat).

    motdet::Motion_detector::Detection contours; // Storage for detected movement across frames.

    auto video_procesing_start = high_resolution_clock::now(); // Chrono the time it takes to process the input source.
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

        if(frame.empty() || millis_frame < millis_prev_frame) break; // End of input video.

        // Turn the captured frame from RGB to grayscale.
        md::Image<unsigned short, md::original_total> grayscale_input_frame(md::original_width);
        unsigned char *rgb_data = (unsigned char *)frame.data;

        md::uchar_to_bw(rgb_data, frame.total(), grayscale_input_frame);

        md::Motion_detector::Detection detected_result;
        motion_detector.detect_motion(grayscale_input_frame, detected_result);

        // Process the detected movement contours. Start or stop recording accordingly.

        if(recording) std::cout << "[REC] ";
        std::cout << "Got results for " << millis_frame << "." << std::endl;

        if(detected_result.detection_count > 0)
        {
            if(!recording)
            {
                // First movement detected in a while, start recording

                fs::path file("motion_" + std::to_string(recordings_counter) + ".mp4");
                fs::path rec_path = out_recordings_dir / file;

                std::cout << "Motion detected. Recording to " << rec_path << std::endl;
                writer.open(rec_path, codec, fps, stream_res, true); // Color video is assumed.
            }

            millis_last_movement = millis_frame;
            contours = detected_result;
            recording = true;
        }
        else
        {
            if(recording && millis_last_movement + millis_keep_recording < millis_frame)
            {
                // No movement detected in feed after a while, closing recording.

                std::cout << "No motion detected. Closing recording " << recordings_counter << std::endl;
                ++recordings_counter;

                contours.detection_count = 0;
                recording = false;
                writer.release();
            }
        }

        if(recording)
        {
            // Draw the detected motion onto the recovered frame as rectangles.
            for(std::size_t i = 0; i < contours.detection_count; ++i)
            {
                md::Motion_detector::Contour &cont = contours.detections[i];
                // Draw the detected movement on screen

                cv::Point pt1(cont.bb_tl_x, cont.bb_tl_y);
                cv::Point pt2(cont.bb_br_x, cont.bb_br_y);
                cv::rectangle(frame, pt1, pt2, cv::Scalar(0, 255, 0));
            }

            // Save the edited frame into the video we are recording.
            writer.write(frame);
        }

    }

    auto video_procesing_end = high_resolution_clock::now();
    std::cout << "Processed video in " << duration_cast<milliseconds>(video_procesing_end - video_procesing_start).count() << " millis " << std::endl;
    return 0;
}
