#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui_c.h>

#include "SeamCarver.h"

class MultiSize : public SeamCarver
{
private:
    int rows;
    int cols;
    std::vector<std::vector<int>> h_seam_buffer;

public:
    MultiSize() {}
    MultiSize(const char *filename)
    {
        if (!loader.LoadImage(filename, input_img))
        {
            output_img = input_img.clone();
            PreCompute();
            UpdateImage();
        }
    }

    void PreCompute()
    {
        rows = output_img.rows;
        cols = output_img.cols;

        cv::Mat temp_img = output_img.clone();
        for (int i = 0; i < cols - 1; ++i)
        {
            CalculateEnergyMap(temp_img);
            FindSeam(energy_map);
            seam_buffer.push_back(seam);
            temp_img = RemoveSeam(seam, temp_img);
        }

        // 竖着的
        temp_img = output_img.clone();
        temp_img = RotateImage(temp_img);
        for(int i = 0; i < rows - 1; ++i)
        {
            CalculateEnergyMap(temp_img);
            FindSeam(energy_map);
            h_seam_buffer.push_back(seam);
            temp_img = RemoveSeam(seam, temp_img);            
        }

        std::cout << "The input image size is: "
                  << rows << " " << cols << std::endl;
    }

    void UpdateImage()
    {
        std::string name = "Multi-Size Image. Esc to confirm.";

        cv::namedWindow(name, CV_WINDOW_NORMAL);
        cv::imshow(name, input_img);
        cv::resizeWindow(name, cols, rows);
        cv::waitKey(1);

        int tempCols = cols;
        int tempRows = rows;

        while (1)
        {
            auto newSize = cv::getWindowImageRect(name);
            if (newSize.width >= cols * 2 || newSize.height >= rows * 2)
            {
                std::cout << "Invalid size!" << std::endl;
                break;
            }

            if (newSize.width > tempCols)
            {
                output_img = input_img.clone();    
                for (int i = 0; i < newSize.width - cols; ++i)
                {
                    output_img = AddSeam(seam_buffer[i], output_img);
                }
                tempCols = newSize.width;
            }
            else if(newSize.width < tempCols)
            {
                output_img = input_img.clone();   
                for (int i = 0; i < cols - newSize.width; ++i)
                {
                    output_img = RemoveSeam(seam_buffer[i], output_img);
                }
                tempCols = newSize.width;
            }

            if (newSize.height > tempRows)
            {
                output_img = input_img.clone();    
                output_img = RotateImage(output_img); 
                for (int i = 0; i < newSize.height - rows; ++i)
                {
                    output_img = AddSeam(h_seam_buffer[i], output_img);
                }
                output_img = RotateBack(output_img);
                tempRows = newSize.height;
            }
            else if(newSize.height < tempRows)
            {
                output_img = input_img.clone();
                output_img = RotateImage(output_img);   
                for (int i = 0; i < rows - newSize.height; ++i)
                {
                    output_img = RemoveSeam(h_seam_buffer[i], output_img);
                }
                output_img = RotateBack(output_img);
                tempRows = newSize.height;
            }

            cv::namedWindow(name, CV_WINDOW_NORMAL);
            cv::imshow(name, output_img);
            char key = cv::waitKey(1);
            if (key == 27)
                break;
        }
        seam_buffer.clear();
    }
};