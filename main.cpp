#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "camera_grab.h"


#define Cam_H_Size 800
#define Cam_V_Size 600

#define Screen_H_Size 1024
#define Screen_V_Size 768

#define Motion_Window_H_Size Screen_H_Size * .75
#define Motion_Window_V_Size Screen_V_Size * .75

#define Motion_Window_H_Pos Motion_Window_H_Size * .125
#define Motion_Window_V_Pos Motion_Window_V_Size * .125





int main()
{

    bool camera_good;
    auto loopStartTime = std::chrono::steady_clock::now();

    // std::string imageFile = "/home/jim/Desktop/PiTests/images/image.jpg"; // Path to your image file
    std::string imageFile = "../../images/image.jpg"; // Path to your image file

    cv::Mat img = loadImage(imageFile);

    cv::Mat gray_frame(Screen_V_Size, Screen_H_Size, CV_8UC1);     // Create an empty cv::Mat with the desired dimensions
    cv::Mat frame_Abs_Diff(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);                     // Create an empty cv::Mat with the desired dimensions

    cv::VideoCapture cap = InitWebCam(camera_good, Cam_H_Size, Cam_V_Size);

    cv::namedWindow("Test Webcam Feed", cv::WINDOW_AUTOSIZE);

    while (true)
    {

        // get camera frame and noise
        get_camera_frame(cap, gray_frame, frame_Abs_Diff);
        //  get_camera_frame_Test(cap, gray_frame);

        // display the image (not needed in final)
        cv::imshow("Test Webcam Feed", gray_frame);
        // cv::imshow("Test frame_Abs_Diff Feed", frame_Abs_Diff);

        // opencv display stuff
        int key = cv::waitKey(1);
        if (key == 27) // ASCII code for the escape key
            break;

        // timing
        auto loopEndTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        // std::cout << "Loop duration: " << elapsed_seconds.count() << "s\n";
        if (elapsed_seconds.count() > .015)
            std::cout << "Loop duration BIGGGGGGGGGGGGGGGGGGGGGGG: " << elapsed_seconds.count() << "s\n";
        loopStartTime = std::chrono::steady_clock::now();
    }

    return 0;
}
