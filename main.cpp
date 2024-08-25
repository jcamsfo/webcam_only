#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <sys/select.h>
#include <unistd.h>
#include <filesystem>
#include <regex>
#include <thread>

#include <opencv2/opencv.hpp>

#include "camera_grab.h"



// #include <boost/filesystem.hpp>


// namespace fs = boost::filesystem;


// #define Cam_H_Size 800
// #define Cam_V_Size 600

// #define Screen_H_Size 1024
// #define Screen_V_Size 768

// #define Motion_Window_H_Size Screen_H_Size * .75
// #define Motion_Window_V_Size Screen_V_Size * .75

namespace fs = std::filesystem;

struct Client_Parameters_Main
{
    int Cam_H_Size;
    int Cam_V_Size;
    int Screen_H_Size;
    int Screen_V_Size;

    int Motion_Window_H_Size_Multiplier;
    int Motion_Window_V_Size_Multiplier;

    float Cycle_Time;

    int Noise_Threshold;
    int Motion_Threshold;

    std::string param3;
    // std::string param4;

    // Constructor to initialize default values
    Client_Parameters_Main() : Cam_H_Size(800), Cam_V_Size(600),
                               Screen_H_Size(1024), Screen_V_Size(768),
                               Motion_Window_H_Size_Multiplier(75), Motion_Window_V_Size_Multiplier(75), Cycle_Time(1.2), Noise_Threshold(5), Motion_Threshold(5000) {}

    int Motion_Window_H_Size = (Screen_H_Size * Motion_Window_H_Size_Multiplier) / 100;
    int Motion_Window_V_Size = (Screen_V_Size * Motion_Window_V_Size_Multiplier) / 100;

    int Motion_Window_H_Position = (Screen_H_Size - Motion_Window_H_Size) / 2;
    int Motion_Window_V_Position = (Screen_V_Size - Motion_Window_V_Size) / 2;
};

void readParametersFromFile(const std::string &filename, Client_Parameters_Main &params)
{
    // Open the file containing the parameters
    std::ifstream infile(filename);
    if (!infile)
    {
        std::cerr << "Error opening file" << std::endl;
        return;
    }

    // Read the file line by line
    std::string line;
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
        std::string name;
        std::string value;

        if (!(iss >> name >> value))
        {
            continue; // Skip lines that don't match the expected format
        }

        // Update the corresponding parameter in the structure
        if (name == "Cam_H_Size")
        {
            params.Cam_H_Size = std::stoi(value); // Convert string to integer
        }
        else if (name == "Cam_V_Size")
        {
            params.Cam_V_Size = std::stoi(value); // Convert string to integer
        }

        else if (name == "Screen_H_Size")
        {
            params.Screen_H_Size = std::stoi(value); // Convert string to integer
        }
        else if (name == "Screen_V_Size")
        {
            params.Screen_V_Size = std::stoi(value); // Convert string to integer}
        }

        else if (name == "Motion_Window_H_Size_Multiplier")
        {
            params.Motion_Window_H_Size_Multiplier = std::stoi(value); // Convert string to integer
        }
        else if (name == "Motion_Window_V_Size_Multiplier")
        {
            params.Motion_Window_V_Size_Multiplier = std::stoi(value); // Convert string to integer
        }
        else if (name == "Cycle_Time")
        {
            params.Cycle_Time = std::stof(value); // Convert string to integer
        }
        else if (name == "Noise_Threshold")
        {
            params.Noise_Threshold = std::stoi(value); // Convert string to integer
        }
        else if (name == "Motion_Threshold")
        {
            params.Motion_Threshold = std::stoi(value); // Convert string to integer
        }
        else if (name == "String_Test")
        {
            params.param3 = value; // String value
        }

        params.Motion_Window_H_Size = (params.Screen_H_Size * params.Motion_Window_H_Size_Multiplier) / 100;
        params.Motion_Window_V_Size = (params.Screen_V_Size * params.Motion_Window_V_Size_Multiplier) / 100;

        params.Motion_Window_H_Position = (params.Screen_H_Size - params.Motion_Window_H_Size) / 2;
        params.Motion_Window_V_Position = (params.Screen_V_Size - params.Motion_Window_V_Size) / 2;
    }
}

std::string getNextFileName(const std::string& directory) {
    std::regex filePattern(R"((\d+)\.raw)");

    int maxNumber = -1;

    // Iterate through the files in the directory
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry)) {
            std::string filename = entry.path().filename().string();
            std::smatch match;
            if (std::regex_match(filename, match, filePattern)) {
                int number = std::stoi(match[1]);
                if (number > maxNumber) {
                    maxNumber = number;
                }
            }
        }
    }

    // Determine the new file name
    int newFileNumber = maxNumber + 1;
    std::string newFilename = std::to_string(newFileNumber).insert(0, 6 - std::to_string(newFileNumber).length(), '0') + ".raw";

    return directory + newFilename;
}



void writeMatRawData(const cv::Mat &mat, const std::string &filename)
{
    // Open a binary file for writing
    std::ofstream outFile(filename, std::ios::binary);

    // Check if the file is opened successfully
    if (!outFile)
    {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    // Write matrix data (no additional metadata)
    outFile.write(reinterpret_cast<const char *>(mat.data), mat.total() * mat.elemSize());

    // Close the file
    outFile.close();
}




#define Minimum_Time_Between_Storing_Images 1

void Sequencer(const bool Image_Motion, const cv::Mat &gray_frame_local)
{
    static std::time_t currentTime = std::time(nullptr);
    static std::tm *localTime = std::localtime(&currentTime);
    static unsigned long last_image_stored = 0;
    static unsigned long time_since_last_iage_stored = 0;

    // std::cout << "Current time in 24-hour format: "
    //           << std::put_time(localTime, "%H:%M:%S")
    //           << " "  << currentTime <<  std::endl;

    currentTime = std::time(nullptr);
    time_since_last_iage_stored = currentTime - last_image_stored;
    if ((time_since_last_iage_stored > 10) && Image_Motion)
    {
        last_image_stored = currentTime;


        auto loopStartTime = std::chrono::steady_clock::now();

        std::string nextFileName = getNextFileName("./raw/");
        writeMatRawData(gray_frame_local, nextFileName);

        auto loopEndTime = std::chrono::steady_clock::now();

        std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        // std::cout << "Loop duration: " << elapsed_seconds.count() << "s\n";

        std::cout << "WRITING next filename " << nextFileName <<  "  time  "  << elapsed_seconds.count() << std::endl;
    }
}

int main()
{

    bool camera_good;
    auto loopStartTime = std::chrono::steady_clock::now();
    bool displayImage = true;
    bool displayMotion = true;
    bool Image_Motion = false;
    bool Image_Motion_Delayed = false;
    long loopCount=0;

    Client_Parameters_Main Client_Params;

    readParametersFromFile("memory_params2.txt", Client_Params);

    std::cout << "Parameter 1: " << Client_Params.Cam_H_Size << std::endl;
    std::cout << "Parameter 2: " << Client_Params.Cam_V_Size << std::endl;
    std::cout << "Parameter 3: " << Client_Params.Screen_H_Size << std::endl;
    std::cout << "Parameter 4: " << Client_Params.Screen_V_Size << std::endl;
    std::cout << "Parameter 5: " << Client_Params.Motion_Window_H_Size << std::endl;
    std::cout << "Parameter 6: " << Client_Params.Motion_Window_V_Size << std::endl;
    std::cout << "Parameter 7: " << Client_Params.Motion_Window_H_Position << std::endl;
    std::cout << "Parameter 8: " << Client_Params.Motion_Window_V_Position << std::endl;
    std::cout << "Parameter 8: " << Client_Params.Cycle_Time << std::endl;
    std::cout << "Parameter 8: " << Client_Params.param3 << std::endl;

    // Sequencer();

    // exit(0);

    std::string imageFile = "../../images/image.jpg"; // Path to your image file

    cv::Mat img = loadImage(imageFile);

    cv::Mat gray_frame(Client_Params.Screen_V_Size, Client_Params.Screen_H_Size, CV_8UC1);                   // Create an empty cv::Mat with the desired dimensions
    cv::Mat frame_Abs_Diff(Client_Params.Motion_Window_V_Size, Client_Params.Motion_Window_H_Size, CV_8UC1); // Create an empty cv::Mat with the desired dimensions

    cv::VideoCapture cap = InitWebCam(camera_good, Client_Params.Cam_H_Size, Client_Params.Cam_V_Size);

    cv::namedWindow("Test Webcam Feed", cv::WINDOW_AUTOSIZE);

    std::cout << " HERR " << getNextFileName("./raw/") << std::endl;

    while (true)
    {

        // get camera frame and noise
        Image_Motion_Delayed = Image_Motion;
        Image_Motion = get_camera_frame(cap, gray_frame, frame_Abs_Diff, Client_Params.Cycle_Time, Client_Params.Motion_Window_H_Position, Client_Params.Motion_Window_V_Position, Client_Params.Noise_Threshold, Client_Params.Motion_Threshold);

        //  get_camera_frame_Test(cap, gray_frame);

        // display the image (not needed in final)
        if (displayImage)
        {
            cv::imshow("Test Webcam Feed", gray_frame);
        }
        else if (cv::getWindowProperty("Test Webcam Feed", cv::WND_PROP_AUTOSIZE) != -1)
        {
            cv::destroyWindow("Test Webcam Feed"); // Close the window if the image is not displayed
        }

        // display the image (not needed in final)
        if (displayMotion)
        {
            cv::imshow("Test frame_Abs_Diff Feed", frame_Abs_Diff);
        }
        else if (cv::getWindowProperty("Test frame_Abs_Diff Feed", cv::WND_PROP_AUTOSIZE) != -1)
        {
            cv::destroyWindow("Test frame_Abs_Diff Feed"); // Close the window if the image is not displayed
        }

        // cv::imshow("Test Webcam Feed", gray_frame);
        // cv::imshow("Test frame_Abs_Diff Feed", frame_Abs_Diff);

        // opencv display stuff
        int key = cv::waitKey(1);
        if (key == 27) // ASCII code for the escape key
            break;

        if (key == 'i')
        {
            displayImage = !displayImage; // Toggle the flag
        }

        if (key == 'm')
        {
            displayMotion = !displayMotion; // Toggle the flag
        }



        Sequencer(Image_Motion_Delayed, gray_frame);

        // if(Image_Motion)
        //   std::cout << "MAIN  Image_Motion " << Image_Motion <<   std::endl;

        // timing

        // std::thread worker(fileHandlingThread, std::cref(gray_frame), directory);

        auto loopEndTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = loopEndTime - loopStartTime;
        // std::cout << "Loop duration: " << elapsed_seconds.count() << "s\n";
        if (elapsed_seconds.count() > .03333)
            std::cout << "Loop duration BIGGGGGGGGGGGGGGGGGGGGGGG: " << elapsed_seconds.count() << "s\n";
        
        if(Image_Motion || Image_Motion_Delayed)
            std::cout << "Loop duration XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxxx: " << elapsed_seconds.count() << "s\n";            
        loopStartTime = std::chrono::steady_clock::now();
        loopCount++;
    }

    return 0;
}
