#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui_c.h>

#include "ImageLoader.h"

typedef uint8_t dtype;

enum Mod
{
    VERTICAL_CARVE,
    HORIZONTAL_CARVE,
    VERTICAL_ENLARGE,
    HORIZONTAL_ENLARGE,
    CONTENT_ENHANCEMENT,
    OBJECT_REMOVAL,
    MULTI_SIZE_IMAGE
};

class SeamCarver
{
protected:
    cv::Mat input_img;
    cv::Mat energy_map;
    cv::Mat output_img;

    ImageLoader loader;

    std::vector<int> seam;
    std::vector<std::vector<int>> seam_buffer;

public:
    SeamCarver() {}
    SeamCarver(const char *filename)
    {
        loader.LoadImage(filename, input_img);
        output_img = input_img.clone();
    }
    ~SeamCarver() {}

    // Base
    void Show()
    {
        cv::destroyAllWindows();
        cv::imshow("input", input_img);
        cv::imshow("output", output_img);
        cv::waitKey(0);
    }
    
    void CalGradient(const cv::Mat &gray_img, cv::Mat &dst, bool dx, bool dy);
    void CalculateEnergyMap(const cv::Mat img);

    cv::Mat RotateImage(const cv::Mat img);
    cv::Mat RotateBack(const cv::Mat rotated_img);

    void FindSeam(const cv::Mat energy_map);
    cv::Mat RemoveSeam(std::vector<int> seam, cv::Mat image);
    cv::Mat AddSeam(std::vector<int> seam, cv::Mat image);

    // Carve Methods
    void VerticalChangeAspectRatio(int size);
    void HorizontalChangeAspectRatio(int size);
    void VerticalImageEnlarging(int size);
    void HorizontalImageEnlarging(int size);
    void ContentEnhancement(int size);

    void OptimalOrder(int size);
    int FindEnergy(const cv::Mat energy_map);
};