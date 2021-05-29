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
        " - output: Folder to store captured video in. Will create .mp4 files with date when motion is detected. NOTE: All files are deleted in the directory." << std::endl <<
        " [OPTIONAL] - threads : Amount of threads, by defaul, 1." << std::endl <<
        " [OPTIONAL] - downscale factor : Resolution reductor for input video, 4 is fastest while still working. 1 is native resolution." << std::endl <<
        " [OPTIONAL] - display stats : Print timing stats. 1 for true, 0 for false" << std::endl;

        return 0;
    }

    std::string in_source;
    std::string out_dir;
    unsigned int threads = 1;
    unsigned int reduction_factor = 1;
    bool display_stats = false;

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
    fs::remove_all(out_recordings_dir);   // Clear the output directory.

    // Get threads
    try { if(argc >= 4) threads = std::abs(std::stoi(argv[3])); }
    catch(const std::exception &e) throw std::invalid_argument("The number of threads must be a number.");
    if(threads == 0) throw std::invalid_argument("The number of threads must be at least 1");

    // Get reduction factor
    try { if(argc >= 5) reduction_factor = std::abs(std::stoi(argv[4])); }
    catch(const std::exception &e) throw std::invalid_argument("The reduction factor must be a number.");
    if(reduction_factor == 0) throw std::invalid_argument("The reduction factor must be at least 1");

    // Get display stats
    if(argc >= 6) display_stats = std::stoi(argv[5]) == 1 ? true : false;

    // At this point, all input parameters have been validated and stored.

    cv::VideoCapture cap;

    // Open either a connected camera or an input video.
    if(input_param == "camera")
    {
        int device_id = 0;             // 0 = open default camera
        int api_id = cv::CAP_ANY;      // 0 = autodetect default API
        cap.open(device_id, api_id);
        std::cout << "Using camera input..." << std::endl;
    }
    else
    {
        cap.open(input_param);
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
    md::Motion_detector motion_detector(width, height, threads, threads*2, reduction_factor);

    // We will record for a few extra seconds after motion is no longer detected to avoid videos turnign off and on intermittently.
    unsigned long long millis_keep_recording = 3000;
    unsigned long long millis_last_movement = 0;
    unsigned long long millis_prev_frame = 0, millis_frame = 0;
    unsigned int recordings_counter = 0; // We will create a separate video for each time movement is detected, use a counter to sequence videos.
    bool recording = false, input_video_ended = false;

    if(display_stats) auto t_video0 = high_resolution_clock::now();

    // Start grabbing frames and checking for motion. Store the captured frame is a OpenCV Matrix (Mat).

    cv::Mat frame;
    cap.read(frame); // Read the first frame outside the loop so that it is lost because the first frame can be corrupted (timestamp wise).
    while(true)
    {
        // Only keep grabbing new frames as long as the input source has frames. A camera will not run out, but a video will.
        if(!input_video_ended)
        {
            // Wait for a new frame and store it into 'frame'
            cap.read(frame);
            millis_prev_frame = millis_frame;
            millis_frame = cap.get(cv::CAP_PROP_POS_MSEC);

            if(frame.empty() || millis_frame < millis_prev_frame)
            {
                std::cout << "Reached end of video, waiting for all frames to be processed..." << std::endl;
                input_video_ended = true;
            }
            else
            {
                // Turn the captured frame from RGB to grayscale. First create a new grayscale image wrapped in a std::unique_ptr
                auto grayscale_input_frame = std::make_unique<md::Image<unsigned short>>(width, height, 0);

                // Get the pointer to the input RGB data, and make the conversion from the rgb to grayscale.
                unsigned char *rgb_data = (unsigned char *)frame.data;
                md::uchar_to_bw(rgb_data, frame.total(), *grayscale_input_frame.get());

                // Enqueue this new grayscale image to the motion detector.
                // Note that moving a unique_ptr to a parameter will transfer the ownership of the data contained away forever.
                // It is not the reponsability of the motion detector to deal with it.
                motion_detector.enqueue_frame(std::move(grayscale_input_frame), millis_frame, true); // Blocks until the frame is submittable
            }
        }
        else
        {
            // Once the video is ended, we need to keep looping until the queued tasks are done.
            if(motion_detector.get_task_queue_size() == 0 && motion_detector.get_result_queue_size() == 0) break;
        }

        // Now poll for results and if there are, check whether we need to start recording or not.

        md::Motion_detector::Detection detected_result;
        bool result_available = true;

        try{ detected_result = motion_detector.get_detection(false); } // Non-blocking result request
        catch(const std::runtime_error &e){ result_available = false; };

        if(result_available)
        {
            // Process the detected movement contours. Start or stop recording accordingly.
            std::cout << "Got results for " << detected_result.first << std::endl;

            if(!detected_result.second.empty())
            {
                if(!recording)
                {
                    // First movement detected in a while, start recording

                    fs::path file("motion_" + std::to_string(recordings_counter) + ".mp4");
                    fs::path rec_path = rec_dir / file;

                    std::cout << "Motion detected. Recording to " << rec_path << std::endl;
                    writer.open(rec_path, codec, fps, stream_res, true); // Color video is assumed.
                }

                millis_last_movement = detected_result.first;
                contours = detected_result.second;
                recording = true;
            }
            else
            {
                if(recording && millis_last_movement + millis_keep_recording < detected_result.first)
                {
                    // No movement detected in feed after a while, closing recording.

                    std::cout << "No motion detected. Closing recording " << recordings_counter << std::endl;
                    ++recordings_counter;

                    contours.clear();
                    recording = false;
                    writer.release();
                }
            }

            if(recording)
            {
                for(md::Contour &cont : contours)
                {
                    // Draw the detected movement on screen

                    cv::Point pt1(cont.bb_tl_x, cont.bb_tl_y);
                    cv::Point pt2(cont.bb_br_x, cont.bb_br_y);
                    cv::rectangle(frame, pt1, pt2, cv::Scalar(0, 255, 0));
                }
                writer.write(frame);
            }
        }
    }

    auto t_video1 = high_resolution_clock::now();
    std::cout << "Processed video in " << duration_cast<milliseconds>(t_video1 - t_video0).count() << " millis " << std::endl;
    return 0;
}
