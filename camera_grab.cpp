#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "camera_grab.h"

// Load a grayscale image
cv::Mat loadImage(const std::string &imageFile)
{
    cv::Mat img = cv::imread(imageFile, cv::IMREAD_GRAYSCALE);
    if (img.empty())
    {
        std::cerr << "Couldn't open image " << imageFile << ".\n";
        exit(-1);
    }
    return img;
}

cv::VideoCapture InitWebCam(bool &valid_cam, int Horizontal_Res, int Vertical_Res) // bool init, cv::Mat gray_frame)
{
    cv::VideoCapture capture(0);
    if (!capture.isOpened())
    {
        printf("Error: Could not open the webcam.\n");
        valid_cam = false;
        return capture;
    }
    else
    {
        printf("Webcam open successful \n");
        valid_cam = true;
        capture.set(cv::CAP_PROP_FRAME_WIDTH, Horizontal_Res);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, Vertical_Res);
        return capture;
    }
}

bool get_camera_frame(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame,
                      const float Cycle_Time_In, const int Motion_H_Pos, const int Motion_V_Pos,
                      const int Noise_Thresh, const int Motion_Thresh)
{
    static int loop_count = 418;

    static auto Sample_Time_Stored = std::chrono::steady_clock::now();
    static auto Sample_Time_Current = std::chrono::steady_clock::now();
    static std::chrono::duration<double> Cycle_Time = Sample_Time_Current - Sample_Time_Stored;
    static bool full_cycle = false;

    static cv::Mat frame_local; // (768, 1024, CV_8UC1);
    static cv::Mat prev_frame;  // (768, 1024, CV_8UC1);

    static cv::Mat masked_frame_previous;
    static cv::Mat masked_frame_current;

    static cv::Scalar sum_diff;

    static bool Image_Motion = false;

    int diff_frame_width = diff_frame.cols;
    int diff_frame_height = diff_frame.rows;

    masked_frame_previous.create(diff_frame.rows, diff_frame.cols, CV_8UC1);
    masked_frame_current.create(diff_frame.rows, diff_frame.cols, CV_8UC1);

    int screen_width = gray_frame_local.cols;
    int screen_height = gray_frame_local.rows;

    static long nonZeroCount;

    cv::Rect roi(Motion_H_Pos, Motion_V_Pos, diff_frame_width, diff_frame_height);

    loop_count++;
    if (loop_count >= 440)
        loop_count = 0;

    Sample_Time_Current = std::chrono::steady_clock::now();
    Cycle_Time = Sample_Time_Current - Sample_Time_Stored;

    Image_Motion = false;

    // wait for half the cycle time to capture a frame
    if (Cycle_Time.count() >= Cycle_Time_In / 2)
    {
        Sample_Time_Stored = Sample_Time_Current;

        auto loopStartTime = std::chrono::steady_clock::now();

        capture >> frame_local; // // Camera Size Color
        // If the frame is empty, break the loop
        if (frame_local.empty())
        {
            printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
            // break;
        }

        // store previous image for motion detect  (not sent to the screen)
        if (full_cycle)
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, prev_frame, cv::COLOR_BGR2GRAY); // Convert to BW   Camera Size BW

            // Resize the grayscale camera frame to Screen Size BW
            cv::resize(prev_frame, prev_frame, cv::Size(screen_width, screen_height));

            // Create smaller image for detecting motion in the central 75%
            masked_frame_previous = prev_frame(roi).clone(); // Create Masked frame for motion detect  Masked Size BW
        }
        else
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, gray_frame_local, cv::COLOR_BGR2GRAY);

            // Resize the grayscale camera frame to Screen Size BW  screen output
            cv::resize(gray_frame_local, gray_frame_local, cv::Size(screen_width, screen_height));

            if (!prev_frame.empty())
            {
                // Create smaller image for detecting motion in the central 75%
                masked_frame_current = gray_frame_local(roi).clone();

                // caclulate the absoute difference of the previousframe and current frame
                cv::absdiff(masked_frame_current, masked_frame_previous, diff_frame);

                diff_frame -= Noise_Thresh;
                // diff_frame *= 10;

                // sum the number of pixels that have moved and divide by 255
                sum_diff = cv::sum(diff_frame);

                nonZeroCount = cv::countNonZero(diff_frame);

                if (nonZeroCount > Motion_Thresh)
                    Image_Motion = true;
            }
        }

        // timing measure
        auto loopEndTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        std::cout << "camera stuff " << full_cycle << "  " << elapsed_seconds.count() << "  " << sum_diff << "   nonZeroCount " << nonZeroCount << "\n";
        // std::cout << " " << Image_Motion << "\n" ;

        full_cycle = !full_cycle;
    }

    // if(Image_Motion)
    //     std::cout << "Image_Motion " << Image_Motion << "nonZeroCount " << nonZeroCount <<  std::endl;

    return Image_Motion;
}

int get_camera_frame2(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame,
                      const float Cycle_Time_In, const int Motion_H_Pos, const int Motion_V_Pos,
                      const int Noise_Thresh, const int Motion_Thresh)
{
    static int Image_Status = -1;
    static int loop_count = 0;

    static auto Sample_Time_Stored = std::chrono::steady_clock::now();
    static auto Sample_Time_Current = std::chrono::steady_clock::now();
    static std::chrono::duration<double> Cycle_Time = Sample_Time_Current - Sample_Time_Stored;
    static bool full_cycle = false;

    static cv::Mat Before_After_Frame; // (768, 1024, CV_8UC1);
    static cv::Mat Main_Frame;         // (768, 1024, CV_8UC1);

    std::vector<cv::Mat> mats;

    static cv::Mat Before_After_Frame_Masked;
    static cv::Mat Main_Frame_Masked;
    static cv::Mat Diff_Frame_Temp;

    static cv::Scalar sum_diff_12, sum_diff_23;

    static long nonZeroCount_12, nonZeroCount_23;

    int diff_frame_width = diff_frame.cols;
    int diff_frame_height = diff_frame.rows;

    Before_After_Frame_Masked.create(diff_frame.rows, diff_frame.cols, CV_8UC1);
    Main_Frame_Masked.create(diff_frame.rows, diff_frame.cols, CV_8UC1);

    Diff_Frame_Temp.create(diff_frame.rows, diff_frame.cols, CV_8UC1);

    int screen_width = gray_frame_local.cols;
    int screen_height = gray_frame_local.rows;

    cv::Rect roi(Motion_H_Pos, Motion_V_Pos, diff_frame_width, diff_frame_height);

    Sample_Time_Current = std::chrono::steady_clock::now();
    Cycle_Time = Sample_Time_Current - Sample_Time_Stored;

    Image_Status = -1;

    // wait for half the cycle time to capture a frame
    if ((Cycle_Time.count() >= Cycle_Time_In) && (loop_count <= 7))
    {

        auto loopStartTime = std::chrono::steady_clock::now();

        if ((loop_count == 0) || (loop_count == 6)) // capture the frame 3 frames before and 3 frames after
        {
            capture >> Before_After_Frame;
            cv::cvtColor(Before_After_Frame, Before_After_Frame, cv::COLOR_BGR2GRAY);                  // Convert to BW   Camera Size BW
            cv::resize(Before_After_Frame, Before_After_Frame, cv::Size(screen_width, screen_height)); // resize the motion detector frame
            Before_After_Frame_Masked = Before_After_Frame(roi).clone();                               // Create Masked frame for motion detect  Masked Size BW
        }
        else if (loop_count == 3) // capture the main frame which is the one displayed
        {
            capture >> Main_Frame;
            cv::cvtColor(Main_Frame, Main_Frame, cv::COLOR_BGR2GRAY); // Convert to BW   Camera Size BW
            cv::resize(Main_Frame, Main_Frame, cv::Size(screen_width, screen_height));
            Main_Frame_Masked = Main_Frame(roi).clone(); // Create Masked frame for motion detect  Masked Size BW

            std::cout << "nonZeroCount_12 " << nonZeroCount_12 << " nonZeroCount_34 " << nonZeroCount_23 << std::endl;
        }

        else if (loop_count == 4)
        {
            cv::absdiff(Before_After_Frame_Masked, Main_Frame_Masked, Diff_Frame_Temp);

            cv::medianBlur(Diff_Frame_Temp, Diff_Frame_Temp, 3);

            Diff_Frame_Temp -= Noise_Thresh;

            diff_frame = Diff_Frame_Temp.clone();

            sum_diff_12 = cv::sum(Diff_Frame_Temp); // sum the number of pixels that have moved and divide by 255

            nonZeroCount_12 = cv::countNonZero(Diff_Frame_Temp);
        }

        else if (loop_count == 7)
        {
            cv::absdiff(Before_After_Frame_Masked, Main_Frame_Masked, Diff_Frame_Temp);

            cv::medianBlur(Diff_Frame_Temp, Diff_Frame_Temp, 3);

            Diff_Frame_Temp -= Noise_Thresh;

            sum_diff_23 = cv::sum(Diff_Frame_Temp); // sum the number of pixels that have moved and divide by 255

            nonZeroCount_23 = cv::countNonZero(Diff_Frame_Temp);

            if ((nonZeroCount_12 > Motion_Thresh) && (nonZeroCount_23 > Motion_Thresh))
            {
                Image_Status = 1;
                std::cout << "nonZeroCount_12 " << nonZeroCount_12 << " nonZeroCount_34 " << nonZeroCount_23 << std::endl;
            }
            else
                Image_Status = 0;

            gray_frame_local = Main_Frame.clone();
        }

        // timing measure
        // if ((loop_count != 1) && (loop_count != 2) && (loop_count != 5))
        // {
        //     auto loopEndTime = std::chrono::steady_clock::now();
        //     std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        //     std::cout << "camera stuff " << full_cycle << "   loopcnt " << loop_count << "   "  <<  elapsed_seconds.count() << "  " << sum_diff_12 << "   nonZeroCount " << nonZeroCount_12 << "\n";
        // }

        loop_count++;
        if (loop_count >= 8)
        {
            Sample_Time_Stored = Sample_Time_Current;
            loop_count = 0;
        }
    }

    // -1 = do nothing   0 = new image    1 = motion
    return Image_Status;
}
