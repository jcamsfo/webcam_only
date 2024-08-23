
#ifndef CAMERA_GRAB_H
#define CAMERA_GRAB_H

#include <opencv2/opencv.hpp>



// Loads a grayscale image
cv::Mat loadImage(const std::string &imageFile);

cv::VideoCapture InitWebCam(bool &valid_cam, int Horizontal_Res, int Vertical_Res);

void get_camera_frame(cv::VideoCapture &capture, cv::Mat &gray_frame_local, cv::Mat &diff_frame, float Cycle_Time_In);

void get_camera_frame_Test(cv::VideoCapture &capture, cv::Mat &gray_frame_local);

#endif // CAMERA_GRAB_H