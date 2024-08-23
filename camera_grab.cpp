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


void get_camera_frame(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame, float Cycle_Time_In) 
{
    static int loop_count = 418;

    static auto Sample_Time_Stored = std::chrono::steady_clock::now();
    static auto Sample_Time_Current = std::chrono::steady_clock::now();    
    static std::chrono::duration<double> Cycle_Time = Sample_Time_Current - Sample_Time_Stored;
    static bool full_cycle = false;

    static cv::Mat frame_local; // (768, 1024, CV_8UC1); 
    static cv::Mat prev_frame; // (768, 1024, CV_8UC1);

    static cv::Mat masked_frame_previous;
    static cv::Mat masked_frame_current;

    int diff_frame_width = diff_frame.cols;
    int diff_frame_height = diff_frame.rows;    

    masked_frame_previous.create(diff_frame.rows, diff_frame.cols, CV_8UC1); 
    masked_frame_current.create(diff_frame.rows, diff_frame.cols, CV_8UC1); 

    int screen_width = gray_frame_local.cols;
    int screen_height = gray_frame_local.rows;

    cv::Rect roi(screen_width * .125, screen_height * .125, diff_frame_width, diff_frame_height);


    loop_count++;
    if (loop_count >= 440)
        loop_count = 0;

    Sample_Time_Current = std::chrono::steady_clock::now();  
    Cycle_Time = Sample_Time_Current - Sample_Time_Stored;

    // wait for half the cycle time to capture a frame
    if(Cycle_Time.count() >= Cycle_Time_In/2 )
    {
        Sample_Time_Stored = Sample_Time_Current;

        auto loopStartTime = std::chrono::steady_clock::now();

        capture >> frame_local;                     // // Camera Size Color
        // If the frame is empty, break the loop
        if (frame_local.empty())
        {
            printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
            // break;
        }

        // store previous image for motion detect  (not sent to the screen)
        if(full_cycle)
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, prev_frame, cv::COLOR_BGR2GRAY);      // Convert to BW   Camera Size BW

            // Resize the grayscale camera frame to Screen Size BW
            cv::resize(prev_frame, prev_frame, cv::Size(screen_width, screen_height));      

            // Create smaller image for detecting motion in the central 75%
            masked_frame_previous = prev_frame(roi).clone();                // Create Masked frame for motion detect  Masked Size BW


            // masked_frame_previous.setTo(cv::Scalar(0, 0, 0)); 

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

                diff_frame -= 5;
                // diff_frame *= 10;

                // threshold the absolute difference and set to 255 for seeing on the screen
             //   cv::threshold(diff_frame, diff_frame, 40, 255, cv::THRESH_BINARY);

                // sum the number of pixels that have moved and divide by 255
                cv::Scalar sum_diff = cv::sum(diff_frame) / 255;

                // timing measure
                auto loopEndTime = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
                std::cout << "SUMMMMMMMMMMMMMMMMMMMMMMMMMMM " << elapsed_seconds.count() << "  " << sum_diff << "s\n";
            }
        }
        full_cycle = !full_cycle ;
    }
}


// void get_camera_frame_Test(cv::VideoCapture &capture, cv::Mat &gray_frame_local) // bool init, cv::Mat gray_frame)
// {
//     static int loop_count = 418;

//     static auto Sample_Time_Stored = std::chrono::steady_clock::now();
//     static auto Sample_Time_Current = std::chrono::steady_clock::now();    
//     static std::chrono::duration<double> Cycle_Time = Sample_Time_Current - Sample_Time_Stored;
//     static bool full_cycle = false;

//     static cv::Mat frame_local; // (768, 1024, CV_8UC1); 
//     static cv::Mat prev_frame; // (768, 1024, CV_8UC1);

//     static cv::Mat masked_frame_previous(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);
//     static cv::Mat masked_frame_current(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);

//     cv::Rect roi(Motion_Window_H_Pos, Motion_Window_V_Pos, Motion_Window_H_Size, Motion_Window_V_Size);

//     loop_count++;
//     // std::cout << "loop_cnt " << loop_count << std::endl;

//     if (loop_count >= 440)
//         loop_count = 0;


//     Sample_Time_Current = std::chrono::steady_clock::now();  
//     Cycle_Time = Sample_Time_Current - Sample_Time_Stored;

//     if(Cycle_Time.count() >= Half_Frame_Duration)
//     {
//         Sample_Time_Stored = Sample_Time_Current;   

//        auto loopStartTime = std::chrono::steady_clock::now();

//         capture >> frame_local;                     // // Camera Size Color
//         // If the frame is empty, break the loop
//         if (frame_local.empty())
//         {
//             printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
//             // break;
//         }
//             // Convert to BW   (Camera Size BW)
//             cv::cvtColor(frame_local, gray_frame_local, cv::COLOR_BGR2GRAY);      // Convert to BW   Camera Size BW

//             // Resize the grayscale camera frame to Screen Size BW
//             cv::resize(gray_frame_local, gray_frame_local, cv::Size(Screen_H_Size, Screen_V_Size));      

//         full_cycle = !full_cycle ;
//     }
// }

