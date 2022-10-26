#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

class ImageLoader
{
public:
    ImageLoader() {}

    int LoadImage(const char *filename, cv::Mat &img);
    int SaveImage(const char *filename, cv::Mat &img);
};