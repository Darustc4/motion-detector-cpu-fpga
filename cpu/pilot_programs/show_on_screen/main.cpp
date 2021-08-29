#include <iostream>
#include <cstddef>
#include <filesystem>
#include <cstring>
#include <vector>
#include <chrono>
#include <exception>

#include <motion_detector.hpp>
#include <opencv2/opencv.hpp>

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
        " [OPTIONAL] - threads : Amount of threads, by defaul, 1." << std::endl <<
        " [OPTIONAL] - downscale factor : Resolution reductor for input video, 4 is fastest while still working. 1 is native resolution." << std::endl <<
        " [OPTIONAL] - display stats : Print timing stats. 1 for true, 0 for false" << std::endl;

        return 0;
    }

    std::string in_source;
    unsigned int threads = 1;
    unsigned int reduction_factor = 1;
    bool display_stats = false;

    // Get input source
    in_source = argv[1];
    if(in_source != "camera")
    {
        if(!fs::exists(in_source)) throw std::invalid_argument("The selected input video file is not accessible or does not exist.");
    }

    // Get threads
    try { if(argc >= 4) threads = std::abs(std::stoi(argv[3])); }
    catch(const std::exception &e) { throw std::invalid_argument("The number of threads must be a number."); }
    if(threads == 0) throw std::invalid_argument("The number of threads must be at least 1");

    // Get reduction factor
    try { if(argc >= 5) reduction_factor = std::abs(std::stoi(argv[4])); }
    catch(const std::exception &e) { throw std::invalid_argument("The reduction factor must be a number."); }
    if(reduction_factor == 0) throw std::invalid_argument("The reduction factor must be at least 1");

    // Get display stats
    if(argc >= 6) display_stats = std::stoi(argv[5]) == 1 ? true : false;

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

    unsigned char fps = cap.get(cv::CAP_PROP_FPS);
    std::size_t width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    std::size_t height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size stream_res = cv::Size(width, height);

    // Create the Motion detector object from the library.
    // Specify an input queue 2 times the amount of threads, so that threads are always busy working.
    md::Motion_detector motion_detector(width, height, threads, threads*2, reduction_factor);

    cv::namedWindow("Motion detector", 1);

    unsigned long long millis_keep_conts = 1000;
    unsigned long long millis_last_movement = 0;
    unsigned long long millis_prev_frame = 0, millis_frame = 0;
    bool input_video_ended = false, first_iteration = true;

    if(display_stats) auto t_video0 = high_resolution_clock::now();

    // Start grabbing frames and checking for motion. Store the captured frame is a OpenCV Matrix (Mat).

    std::vector<motdet::Contour> contours; // Storage for detected movement across frames.

    while(true)
    {
        // Only keep grabbing new frames as long as the input source has frames. A camera will not run out, but a video will.
        if(!input_video_ended)
        {
            // Create a container for the new frame that we are going to read, and fill it from the input source.
            std::shared_ptr<cv::Mat> frame = std::make_unique<cv::Mat>();
            cap.read(*frame.get());

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

            if(frame->empty() || millis_frame < millis_prev_frame)
            {
                std::cout << "Reached end of video, waiting for all frames to be processed..." << std::endl;
                input_video_ended = true;
            }
            else
            {
                // Turn the captured frame from RGB to grayscale. First create a new grayscale image wrapped in a std::unique_ptr
                auto grayscale_input_frame = std::make_unique<md::Image<unsigned short>>(width, height, 0);

                // Get the pointer to the input RGB data, and make the conversion from the rgb to grayscale.
                unsigned char *rgb_data = (unsigned char *)frame->data;
                md::uchar_to_bw(rgb_data, frame->total(), *grayscale_input_frame.get());

                // Enqueue this new grayscale image to the motion detector, transfering the ownership of the Image away.
                // We also sent the shared ptr with the frame we read from source.
                // The reason for this is because we want to get it back when polling for results later so that we can
                // use it to record a video if motion is detected.
                motion_detector.enqueue_frame(std::move(grayscale_input_frame), millis_frame, true, frame);
            }
        }
        else
        {
            // Once the video is ended, we need to keep looping until the queued tasks are done.
            if(motion_detector.get_task_queue_size() == 0 && motion_detector.get_result_queue_size() == 0) break;
        }

        md::Detection detected_result;
        bool result_available = true;

        try{ detected_result = motion_detector.get_detection(false); } // Non-blocking result request
        catch(const std::runtime_error &e){ result_available = false; };

        if(result_available)
        {
            // Process the detected movement contours.

            if(detected_result.has_detections)
            {
                millis_last_movement = detected_result.timestamp;
                contours = detected_result.detection_contours;
            }
            else if(millis_last_movement + millis_keep_conts < detected_result.timestamp) contours.clear();

            cv::Mat* recovered_frame = (cv::Mat*)detected_result.data_keep.get();

            // Draw the detected motion onto the recovered frame as rectangles.
            for(md::Contour &cont : contours)
            {
                // Draw the detected movement on screen

                cv::Point pt1(cont.bb_tl_x, cont.bb_tl_y);
                cv::Point pt2(cont.bb_br_x, cont.bb_br_y);
                cv::rectangle(*recovered_frame, pt1, pt2, cv::Scalar(0, 255, 0));
            }

            // Save the edited frame into the video we are recording.
            cv::imshow("Motion detector", *recovered_frame);
            if(cv::waitKey(30) >= 0) break;
        }
    }

    std::cout << "Finished processing video, exiting..." << std::endl;


    return 0;
}
