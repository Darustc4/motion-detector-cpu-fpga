#include <iostream>
#include <cstddef>    // std::size_t
#include <thread>     // std::thread::hardware_concurrency
#include <filesystem> // fs::path
#include <cstring>
#include <vector>
#include <chrono>

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
        " - output: Folder to store captured video in. Will create .mp4 files with date when motion is detected." << std::endl <<
        " [OPTIONAL] - threads : Amount of threads, by defaul, 1." << std::endl <<
        " [OPTIONAL] - downscale factor : Resolution reductor for input video, 4 is fastest while still working. 1 is native resolution." << std::endl <<
        " [OPTIONAL] - debug : Folder to store each middle step image in." << std::endl;

        return -1;
    }

    cv::Mat frame;
    cv::VideoCapture cap;

    std::string input_param = argv[1];
    std::string output_param = argv[2];

    std::string debug_param;
    unsigned int threads = 1;
    unsigned int reduction_factor = 1;

    if(argc >= 4) threads = std::abs(std::stoi(argv[3]));
    if(argc >= 5) reduction_factor = std::abs(std::stoi(argv[4]));

    bool using_debug_mode = true;
    if(argc >= 6) debug_param = argv[5];
    else using_debug_mode = false;

    // Open either a connected camera or an input video.
    if(input_param == "camera")
    {

        int device_id = 0;             // 0 = open default camera
        int api_id = cv::CAP_ANY;      // 0 = autodetect default API

        // Open selected camera using selected API
        cap.open(device_id, api_id);
        std::cout << "Camera started..." << std::endl;

    }
    else
    {
        // Open mp4 file passed as input
        cap.open(input_param);
        std::cout << "MP4 file opened..." << std::endl;
    }

    if (!cap.isOpened())
    {
        std::cerr << "ERROR: Unable to open camera/video" << std::endl;
        return -1;
    }

    // Configure mp4 writer. Video will be recorded when motion is detected.
    cv::VideoWriter writer;
    int codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v'); // Saving to mp4
    unsigned char fps = cap.get(cv::CAP_PROP_FPS);
    std::size_t width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    std::size_t height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size stream_res = cv::Size(width, height);

    fs::path rec_dir(output_param);
    fs::remove_all(rec_dir);   // Clear the output folder.
    fs::create_directory(rec_dir);

    std::cout << "Writing video to directory " << rec_dir << " when motion is detected." << std::endl;

    // Create the Motion detector from the library.
    md::Motion_detector motion_detector(width, height, threads, threads*2, reduction_factor);
    std::vector<md::Contour> contours; // The output of the motion detector.

    if(using_debug_mode) motion_detector.save_debug_images(debug_param);

    // We will record for a few extra seconds after motion is no longer detected to avoid videos turnign off and on intermittently.
    unsigned long long millis_keep_recording = 3000;
    unsigned long long millis_last_movement = 0;
    unsigned long long millis_prev_frame = 0, millis_frame = 0;
    unsigned int recordings_counter = 0; // We will create a separate video for each time movement is detected.

    // If motion is detected, we will start recording to see what is happening.
    bool recording = false, video_ended = false;

    // Start grabbing frames and checking for motion.

    auto t_video0 = high_resolution_clock::now();

    cap.read(frame); // Ignore the first frame since it can be corrupted.
    while(true)
    {
        if(!video_ended)
        {
            // Wait for a new frame and store it into 'frame'
            cap.read(frame);
            millis_prev_frame = millis_frame;
            millis_frame = cap.get(cv::CAP_PROP_POS_MSEC);

            if(frame.empty() || millis_frame < millis_prev_frame)
            {
                std::cout << "Reached end of video, waiting for all frames to be processed..." << std::endl;
                video_ended = true;
                continue;
            }

            // Turn captured frame from RGB to grayscale. First create a new grayscale image wrapped in a std::unique_ptr
            auto input_frame = std::make_unique<md::Image<unsigned short>>(width, height, 0);

            // Get the pointer to the input RGB data, and make the conversion from the rgb to grayscale.
            unsigned char *data = (unsigned char *)frame.data;
            md::uchar_to_bw(data, frame.total(), *input_frame.get());

            // Enqueue this new grayscale image to the motion detector. Passing the unique_ptr by value transfers ownership away, bye frame!
            motion_detector.enqueue_frame(std::move(input_frame), millis_frame, true); // Blocks until the frame is submittable
        }
        else
        {
            // Once the video is ended, we need to keep looping until the queued tasks are done.
            if(motion_detector.get_task_queue_size() == 0 && motion_detector.get_result_queue_size() == 0) break;
        }
        // If the program reached this point, the frame we just got should be submitted.
        // Now poll for results and if there are, check whether we need to start recording or not.

        std::pair<unsigned long long, std::vector<md::Contour>> detected_result;
        bool result_available = true;

        try{ detected_result = motion_detector.get_detected_contours(false); } // Non-blocking result request
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
