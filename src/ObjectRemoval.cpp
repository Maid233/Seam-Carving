#include "ObjectRemoval.h"

void ObjectRemoval::GetMask()
{
    std::string windowTitle = "Draw the removal part. Esc to confirm.";
    cv::Mat backImage;
    cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback(windowTitle, onMouse, (void *)&mask_img);

    while (1)
    {
        if (isDrawing)
        {
            cv::circle(mask_img, currentPoint, 0, cv::Scalar(0, 0, 255), 16);
        }
        cv::addWeighted(input_img, 0.5, mask_img, 0.5, 0, backImage);
        cv::imshow(windowTitle, backImage);
        char key = cv::waitKey(1);
        if (key == 27)
            break;
    }
}

cv::Mat ObjectRemoval::GetEnergyMap(cv::Mat img, cv::Mat mask)
{
    const int rate = -100;

    cv::Mat gray_img, gray_mask, energy_map;
    cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);
    cv::cvtColor(mask, gray_mask, cv::COLOR_BGR2GRAY);

    // Sobel
    cv::Mat sobelX, sobelY;
    cv::Sobel(gray_img, sobelX, CV_16S, 1, 0);
    cv::Sobel(gray_img, sobelY, CV_16S, 0, 1);

    cv::convertScaleAbs(sobelX, sobelX);
    cv::convertScaleAbs(sobelY, sobelY);

    cv::addWeighted(sobelX, 1, sobelY, 1, 0, energy_map, CV_16S);

    cv::threshold(gray_mask, gray_mask, 1, 255, cv::THRESH_BINARY);
    cv::addWeighted(gray_mask, rate, energy_map, 1, 0, energy_map, CV_16S);

    return energy_map;
}

void ObjectRemoval::FindSeam(const cv::Mat energy_map)
{
    const int rows = energy_map.rows;
    const int cols = energy_map.cols;

    std::vector<std::vector<int>> map_data;
    map_data.resize(rows);

    for (int i = 0; i < rows; ++i)
    {
        map_data[i].resize(cols);
    }

    int temp = 1919810;
    int temp_index = -1;
    seam.resize(rows, 0);

    for (int i = 0; i < cols; ++i)
    {
        map_data[0][i] = (int)energy_map.at<short>(0, i);
    }

    for (int i = 1; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            // 边界检测
            if (j == 0)
                map_data[i][j] = std::min(map_data[i - 1][j + 1], map_data[i - 1][j]);
            else if (j == cols - 1)
                map_data[i][j] = std::min(map_data[i - 1][j - 1], map_data[i - 1][j]);
            // 非边界情况
            else
                map_data[i][j] = std::min(map_data[i - 1][j - 1], std::min(map_data[i - 1][j], map_data[i - 1][j + 1]));
            map_data[i][j] += energy_map.at<short>(i, j);
        }
    }

    for (int i = 0; i < cols; ++i)
    {
        // 寻找能量最低的起点
        if (map_data[rows - 1][i] < temp)
        {
            temp = map_data[rows - 1][i];
            temp_index = i;
        }
    }

    seam[rows - 1] = temp_index;
    for (int i = rows - 1; i > 0; --i)
    {
        auto energy = map_data[i][temp_index] - (int)energy_map.at<short>(i, temp_index);
        // 边界检测
        if (temp_index == 0)
        {
            if (energy == map_data[i - 1][temp_index + 1])
                temp_index = temp_index + 1;
            else
                temp_index = temp_index;
        }
        else if (temp_index == cols - 1)
        {
            if (energy == map_data[i - 1][temp_index - 1])
                temp_index = temp_index - 1;
            else
                temp_index = temp_index;
        }
        // 非边界情况
        else
        {
            if (energy == map_data[i - 1][temp_index - 1])
                temp_index = temp_index - 1;
            else if (energy == map_data[i - 1][temp_index + 1])
                temp_index = temp_index + 1;
            else
                temp_index = temp_index;
        }
        seam[i - 1] = temp_index;
    }
}

void ObjectRemoval::UpdateImage()
{
    cv::Mat grayMask;
    uint seamCount = 0;

    if (isVertical)
    {
        output_img = RotateImage(output_img);
        mask_img = RotateImage(mask_img);
    }

    cv::cvtColor(mask_img, grayMask, cv::COLOR_BGR2GRAY);
    while (cv::countNonZero(grayMask) > 0)
    {
        cv::Mat energyMap = GetEnergyMap(output_img, mask_img);
        FindSeam(energyMap);

        cv::Mat carvedImage = output_img.clone();
        for (int i = 0; i < output_img.rows; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                carvedImage.at<cv::Vec3b>(i, seam[i])[j] = 233;
            }
        }
        cv::addWeighted(carvedImage, 0.5, mask_img, 0.5, 0, carvedImage);
        if (isVertical)
            carvedImage = RotateBack(carvedImage);
        cv::imshow("Processing", carvedImage);
        cv::waitKey(1);

        output_img = RemoveSeam(seam, output_img);
        mask_img = RemoveSeam(seam, mask_img);

        cv::cvtColor(mask_img, grayMask, cv::COLOR_BGR2GRAY);
        seamCount++;
    }

    if (resizeToOrigin)
    {
        cv::Mat zeroImage = cv::Mat(output_img.size(), CV_8UC3);
        zeroImage = cv::Scalar(0, 0, 0);
        for (uint i = seamCount; i >= 1; --i)
        {
            cv::Mat energyMap = GetEnergyMap(output_img, zeroImage);
            FindSeam(energyMap);
            cv::Mat carvedImage = output_img.clone();
            for (int j = 0; j < output_img.rows; ++j)
            {
                for (int k = 0; k < 3; ++k)
                {
                    carvedImage.at<cv::Vec3b>(j, seam[j])[k] = 233;
                }
            }
            output_img = AddSeam(seam, output_img);
            zeroImage = AddSeam(seam, zeroImage);

            if (isVertical)
                carvedImage = RotateBack(carvedImage);
            cv::imshow("Processing", carvedImage);
            cv::waitKey(1);
        }
    }

    if (isVertical)
    {
        output_img = RotateBack(output_img);
        mask_img = RotateBack(mask_img);
    }

    cv::imshow("Output", output_img);
    cv::waitKey(0);
}