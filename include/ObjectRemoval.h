#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui_c.h>

#include "SeamCarver.h"

class ObjectRemoval : public SeamCarver
{
public:
    ObjectRemoval() {}
    ObjectRemoval(const char *filename)
    {
        isVertical = false;
        resizeToOrigin = false;
		
        if (!loader.LoadImage(filename, input_img))
        {
			mask_img = cv::Mat(input_img.size(), CV_8UC3);
			mask_img = cv::Scalar(0, 0, 0);
			output_img = input_img.clone();

			GetMask();
			UpdateImage();
		}
    }

public:
    static bool isDrawing;
    cv::Mat mask_img;
    static cv::Point currentPoint;

    bool isVertical;
    bool resizeToOrigin;

    static void onMouse(int event, int x, int y, int flag, void *param)
    {
        cv::Mat &img = *(cv::Mat *)param;

        switch (event)
        {
        case CV_EVENT_MOUSEMOVE:
            currentPoint = cv::Point(x, y);
            break;
        case CV_EVENT_LBUTTONDOWN:
            isDrawing = true;
            currentPoint = cv::Point(x, y);
            break;
        case CV_EVENT_LBUTTONUP:
            isDrawing = false;
            break;
        }
    }

    void FindSeam(const cv::Mat energy_map);
    
    void GetMask();
    cv::Mat GetEnergyMap(cv::Mat img, cv::Mat mask);
    void UpdateImage();
};