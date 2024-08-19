#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>


#define Cam_H_Size 800
#define Cam_V_Size 600

#define Screen_H_Size 1024
#define Screen_V_Size 768

#define Motion_Window_H_Size Screen_H_Size * .75
#define Motion_Window_V_Size Screen_V_Size * .75

#define Motion_Window_H_Pos Motion_Window_H_Size * .125
#define Motion_Window_V_Pos Motion_Window_V_Size * .125


#define Image_Threshold 1

#define Half_Frame_Duration .6




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

// cv::Mat InitWebCam(bool init, cv::Mat gray_frame)
// {
//     if (init)
//     {
//         cv::Mat frame, resized_frame;
//         cv::VideoCapture cap(0);
//         cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
//         cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
//     }

//     if (!cap.isOpened())
//     {
//         printf("Error: Could not open the webcam.\n");
//         return -1;
//     }
//     else
//     {
//         cap >> frame;
//         cv::cvtColor(frame, gray_frame, cv::COLOR_BGR2GRAY);
//         return frame;
//     }
// }

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

void get_camera_frame(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame) // bool init, cv::Mat gray_frame)
{
    static int loop_count = 438;

    static cv::Mat frame_local(768, 1024, CV_8UC1); // Create an empty cv::Mat with the desired dimensions
    static cv::Mat prev_frame(768, 1024, CV_8UC1);





    loop_count++;
    // std::cout << "loop_cnt " << loop_count << std::endl;

    if (loop_count >= 440)
        loop_count = 0;

    if ((loop_count == 439))
    {

        auto loopStartTime = std::chrono::steady_clock::now();

        prev_frame = gray_frame_local.clone();

        capture >> frame_local;
        // If the frame is empty, break the loop
        if (frame_local.empty())
        {
            printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
            // break;
        }

        cv::cvtColor(frame_local, gray_frame_local, cv::COLOR_BGR2GRAY);
        // Resize the grayscale frame to 1024x768
        cv::resize(gray_frame_local, gray_frame_local, cv::Size(1024, 768));

        if (!prev_frame.empty())
        {

            cv::absdiff(gray_frame_local, prev_frame, diff_frame);
            cv::Scalar sum_diff = cv::sum(diff_frame);
            auto loopEndTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
            std::cout << "SUMMMMMMMMMMMMMMMMMMMMMMMMMMM " << elapsed_seconds.count() << "  " << sum_diff << "s\n";
        }
    }
}

void get_camera_frame_2(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame) // bool init, cv::Mat gray_frame)
{
    static int loop_count = 199;

    static cv::Mat frame_local(768, 1024, CV_8UC1); 
    static cv::Mat prev_frame(768, 1024, CV_8UC1);

    loop_count++;
    // std::cout << "loop_cnt " << loop_count << std::endl;

    if (loop_count >= 440)
        loop_count = 0;

    // if ((loop_count == 439) || (loop_count == 419))
    if ((loop_count == 439) || (loop_count == 200))
    {

        auto loopStartTime = std::chrono::steady_clock::now();

        // prev_frame = gray_frame_local.clone();

        capture >> frame_local;
        // If the frame is empty, break the loop
        if (frame_local.empty())
        {
            printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
            // break;
        }

        // if (loop_count == 419)
        if (loop_count == 200)
        {
            cv::cvtColor(frame_local, prev_frame, cv::COLOR_BGR2GRAY);
            // Resize the grayscale frame to 1024x768
            cv::resize(prev_frame, prev_frame, cv::Size(1024, 768));
        }
        else
        {
            cv::cvtColor(frame_local, gray_frame_local, cv::COLOR_BGR2GRAY);
            // Resize the grayscale frame to 1024x768
            cv::resize(gray_frame_local, gray_frame_local, cv::Size(1024, 768));

            if (!prev_frame.empty())
            {
                cv::absdiff(gray_frame_local, prev_frame, diff_frame);
                cv::threshold(diff_frame, diff_frame, 40, 255, cv::THRESH_BINARY);
                cv::Scalar sum_diff = cv::sum(diff_frame) / 255;
                auto loopEndTime = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
                std::cout << "SUMMMMMMMMMMMMMMMMMMMMMMMMMMM " << elapsed_seconds.count() << "  " << sum_diff << "s\n";
            }
        }
    }
}




void get_camera_frame_3(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame) // bool init, cv::Mat gray_frame)
{
    static int loop_count = 418;

    // static auto Sample_Time = std::chrono::steady_clock::now();


    static cv::Mat frame_local; // (768, 1024, CV_8UC1); 
    static cv::Mat prev_frame; // (768, 1024, CV_8UC1);


    static cv::Mat masked_frame_previous(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);
    static cv::Mat masked_frame_current(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);

    cv::Rect roi(Motion_Window_H_Pos, Motion_Window_V_Pos, Motion_Window_H_Size, Motion_Window_V_Size);

    loop_count++;
    // std::cout << "loop_cnt " << loop_count << std::endl;

    if (loop_count >= 440)
        loop_count = 0;

    // if ((loop_count == 439) || (loop_count == 419))
    if ((loop_count == 439) || (loop_count == 200))
    {

        auto loopStartTime = std::chrono::steady_clock::now();

        // prev_frame = gray_frame_local.clone();

        capture >> frame_local;                     // // Camera Size Color
        // If the frame is empty, break the loop
        if (frame_local.empty())
        {
            printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
            // break;
        }


        // store previous image for motion detect  (not sent to the screen)
        // if (loop_count == 419)
        if (loop_count == 200)
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, prev_frame, cv::COLOR_BGR2GRAY);      // Convert to BW   Camera Size BW

            // Resize the grayscale camera frame to Screen Size BW
            cv::resize(prev_frame, prev_frame, cv::Size(Screen_H_Size, Screen_V_Size));      

            // Create smaller image for detecting motion in the central 75%
            masked_frame_previous = prev_frame(roi).clone();                // Create Masked frame for motion detect  Masked Size BW

        }
        else
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, gray_frame_local, cv::COLOR_BGR2GRAY);

            // Resize the grayscale camera frame to Screen Size BW  screen output
            cv::resize(gray_frame_local, gray_frame_local, cv::Size(Screen_H_Size, Screen_V_Size));

            if (!prev_frame.empty())
            {
                // Create smaller image for detecting motion in the central 75%
                masked_frame_current = gray_frame_local(roi).clone(); 

                // caclulate the absoute difference of the previousframe and current frame
                cv::absdiff(masked_frame_current, masked_frame_previous, diff_frame);

                diff_frame -= 5;
                diff_frame *= 10;

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
    }
}




void get_camera_frame_4(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame) // bool init, cv::Mat gray_frame)
{
    static int loop_count = 418;

    static auto Sample_Time_Stored = std::chrono::steady_clock::now();
    static auto Sample_Time_Current = std::chrono::steady_clock::now();    
    static std::chrono::duration<double> Cycle_Time = Sample_Time_Current - Sample_Time_Stored;
    static bool full_cycle = false;

    static cv::Mat frame_local; // (768, 1024, CV_8UC1); 
    static cv::Mat prev_frame; // (768, 1024, CV_8UC1);


    static cv::Mat masked_frame_previous(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);
    static cv::Mat masked_frame_current(Motion_Window_V_Size, Motion_Window_H_Size, CV_8UC1);

    cv::Rect roi(Motion_Window_H_Pos, Motion_Window_V_Pos, Motion_Window_H_Size, Motion_Window_V_Size);

    loop_count++;
    // std::cout << "loop_cnt " << loop_count << std::endl;



    if (loop_count >= 440)
        loop_count = 0;




    // if ((loop_count == 439) || (loop_count == 419))
    // if ((loop_count == 439) || (loop_count == 200))

    Sample_Time_Current = std::chrono::steady_clock::now();  
    Cycle_Time = Sample_Time_Current - Sample_Time_Stored;

    if(Cycle_Time.count() >= Half_Frame_Duration)
    {
        Sample_Time_Stored = Sample_Time_Current;



        auto loopStartTime = std::chrono::steady_clock::now();

        // prev_frame = gray_frame_local.clone();

        capture >> frame_local;                     // // Camera Size Color
        // If the frame is empty, break the loop
        if (frame_local.empty())
        {
            printf("Error: Could not capture frame XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.\n");
            // break;
        }


        // store previous image for motion detect  (not sent to the screen)
        // if (loop_count == 419)
        // if (loop_count == 200)
        if(full_cycle)
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, prev_frame, cv::COLOR_BGR2GRAY);      // Convert to BW   Camera Size BW

            // Resize the grayscale camera frame to Screen Size BW
            cv::resize(prev_frame, prev_frame, cv::Size(Screen_H_Size, Screen_V_Size));      

            // Create smaller image for detecting motion in the central 75%
            masked_frame_previous = prev_frame(roi).clone();                // Create Masked frame for motion detect  Masked Size BW

        }
        else
        {
            // Convert to BW   (Camera Size BW)
            cv::cvtColor(frame_local, gray_frame_local, cv::COLOR_BGR2GRAY);

            // Resize the grayscale camera frame to Screen Size BW  screen output
            cv::resize(gray_frame_local, gray_frame_local, cv::Size(Screen_H_Size, Screen_V_Size));

            if (!prev_frame.empty())
            {
                // Create smaller image for detecting motion in the central 75%
                masked_frame_current = gray_frame_local(roi).clone(); 

                // caclulate the absoute difference of the previousframe and current frame
                cv::absdiff(masked_frame_current, masked_frame_previous, diff_frame);

                diff_frame -= 5;
                diff_frame *= 10;

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

        get_camera_frame_4(cap, gray_frame, frame_Abs_Diff);

        cv::imshow("Test Webcam Feed", gray_frame);
        cv::imshow("Test frame_Abs_Diff Feed", frame_Abs_Diff);

        int key = cv::waitKey(1);
        if (key == 27) // ASCII code for the escape key
            break;

        auto loopEndTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        std::cout << "Loop duration: " << elapsed_seconds.count() << "s\n";
        if (elapsed_seconds.count() > .020)
            std::cout << "Loop duration BIGGGGGGGGGGGGGGGGGGGGGGG: " << elapsed_seconds.count() << "s\n";
        loopStartTime = std::chrono::steady_clock::now();
    }

    return 0;
}
