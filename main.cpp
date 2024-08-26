
#include <chrono>
#include <ctime>

#include <opencv2/opencv.hpp>

#include "camera_grab.h"
#include "file_io.h"
#include "params.h"


int Display_Test_Images(cv::Mat &Image_1, cv::Mat &Image_2)
{
    bool displayImage = true;
    bool displayMotion = true;
    cv::namedWindow("Test Webcam Feed", cv::WINDOW_AUTOSIZE);

    
    // opencv display stuff
    int key = cv::waitKey(1);
    if (key == 27) // ASCII code for the escape key
        return -1;

    if (key == 'i')
    {
        displayImage = !displayImage; // Toggle the flag
    }

    if (key == 'm')
    {
        displayMotion = !displayMotion; // Toggle the flag
    }

    // Debug Display  // display the image (not needed in final)
    if (displayImage)
    {
        cv::imshow("Test Webcam Feed", Image_1);
    }
    else if (cv::getWindowProperty("Test Webcam Feed", cv::WND_PROP_AUTOSIZE) != -1)
    {
        cv::destroyWindow("Test Webcam Feed"); // Close the window if the image is not displayed
    }

    if (displayMotion)
    {
        cv::imshow("Test frame_Abs_Diff Feed", Image_2);
    }
    else if (cv::getWindowProperty("Test frame_Abs_Diff Feed", cv::WND_PROP_AUTOSIZE) != -1)
    {
        cv::destroyWindow("Test frame_Abs_Diff Feed"); // Close the window if the image is not displayed
    }
    return 1;
}



int main()
{
    bool camera_good;
    auto loopStartTime = std::chrono::steady_clock::now();
    auto loopEndTime = std::chrono::steady_clock::now();
    auto ProcessStartTime = std::chrono::steady_clock::now();
    auto ProcessEndTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> ProcessTime = ProcessEndTime - ProcessStartTime;
    bool displayImage = true;
    bool displayMotion = true;
    bool Image_Motion = false;
    int Image_Status = -1;
    long loopCount = 0;
    Client_Parameters_Main Client_Params;

    readParametersFromFile("memory_params2.txt", Client_Params);

    std::string imageFile = "../../images/image.jpg"; // Path to your image file
    cv::Mat img = loadImage(imageFile);

    uchar gray_frame_raw[Client_Params.Screen_H_Size * Client_Params.Screen_V_Size];
    cv::Mat gray_frame(Client_Params.Screen_V_Size, Client_Params.Screen_H_Size, CV_8UC1, gray_frame_raw);   // Create an empty cv::Mat with the desired dimensions
    cv::Mat frame_Abs_Diff(Client_Params.Motion_Window_V_Size, Client_Params.Motion_Window_H_Size, CV_8UC1); // Create an empty cv::Mat with the desired dimensions
    std::vector<cv::Mat> Mats_5;

    cv::VideoCapture cap = InitWebCam(camera_good, Client_Params.Cam_H_Size, Client_Params.Cam_V_Size);

    cv::namedWindow("Test Webcam Feed", cv::WINDOW_AUTOSIZE);

    std::cout << " HERR " << getNextFileNameRaw("./raw/") << std::endl;
    std::cout << " HERR " << getNextFileNameTif("./tif/") << std::endl;

    while (true)
    {

        ProcessStartTime = std::chrono::steady_clock::now();

        // get camera frame and noise  -1 = do nothing 0 is New Image  1 is New Image and Motion
        Image_Status = get_camera_frame(cap, gray_frame, frame_Abs_Diff, Client_Params.Cycle_Time, Client_Params.Motion_Window_H_Position, Client_Params.Motion_Window_V_Position, Client_Params.Noise_Threshold, Client_Params.Motion_Threshold);

        Image_Motion = (Image_Status == 1);

        Sequencer(Image_Motion, gray_frame);

        if (Image_Status >= 0)
        {
            Mats_5.push_back(gray_frame);
            if (Mats_5.size() > 5)
            {
                Mats_5.resize(5);
            }
        }

        ProcessEndTime = std::chrono::steady_clock::now();
        ProcessTime = ProcessEndTime - ProcessStartTime;

        // sets the timing
        loopEndTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        while (elapsed_seconds.count() < .0333333)
            elapsed_seconds = std::chrono::steady_clock::now() - loopStartTime;
        loopStartTime = std::chrono::steady_clock::now();



        if( Display_Test_Images( gray_frame,  frame_Abs_Diff) == -1)
            break;
        

        if (elapsed_seconds.count() > .034)
            std::cout << "Loop duration BIGGGGGGGGGGGGGGGGGGGGGGG: " << elapsed_seconds.count() << "s\n";


        // if(Image_Motion)
        if (ProcessTime.count() > .004)
            std::cout << "ProcessTime: " << ProcessTime.count() << "s\n";

        loopCount++;
    }

    return 0;
}
