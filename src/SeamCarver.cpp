#include "SeamCarver.h"

// 计算能量
void SeamCarver::CalGradient(const cv::Mat &gray_img, cv::Mat &dst, bool dx, bool dy)
{
    const int cols = gray_img.cols, rows = gray_img.rows;
    dst = gray_img.clone();

    int i, j;
    if (dx)
    {
        for (i = 0; i < rows; ++i)
        {
            for (j = 1; j < cols; ++j)
            {
                auto dif = gray_img.at<dtype>(i, j) - gray_img.at<dtype>(i, j - 1);
                dst.at<dtype>(i, j) = dif;
            }
        }
    }
    else if (dy)
    {
        for (j = 0; j < cols; ++j)
        {
            for (i = 1; i < rows; ++i)
            {
                dst.at<dtype>(i, j) = gray_img.at<dtype>(i, j) - dst.at<dtype>(i - 1, j);
            }
        }
    }
}

void SeamCarver::CalculateEnergyMap(const cv::Mat img)
{
    // 先计算灰度图
    cv::Mat gray_img;
    cv::cvtColor(img, gray_img, cv::COLOR_BGR2GRAY);

    // Sobel
    cv::Mat sobelX, sobelY;
    cv::Sobel(gray_img, sobelX, CV_16S, 1, 0);
    cv::Sobel(gray_img, sobelY, CV_16S, 0, 1);

    // CalGradient(gray_img, sobelX,  1, 0);
    // CalGradient(gray_img, sobelY,  0, 1);

    cv::convertScaleAbs(sobelX, sobelX);
    cv::convertScaleAbs(sobelY, sobelY);

    cv::addWeighted(sobelX, 1, sobelY, 1, 0, energy_map);
}

// 转90度
cv::Mat SeamCarver::RotateImage(const cv::Mat img)
{
    cv::Mat rotated_img;
    cv::transpose(img, rotated_img);
    cv::flip(rotated_img, rotated_img, 1);
    return rotated_img;
}

// 转回来
cv::Mat SeamCarver::RotateBack(const cv::Mat rotated_img)
{
    cv::Mat img;
    cv::transpose(rotated_img, img);
    cv::flip(img, img, 0);
    return img;
}

// 找到能量最少的一条Seam
void SeamCarver::FindSeam(const cv::Mat energy_map)
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
        map_data[0][i] = (int)energy_map.at<uchar>(0, i);
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
            map_data[i][j] += energy_map.at<uchar>(i, j);
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
        auto energy = map_data[i][temp_index] - (int)energy_map.at<uchar>(i, temp_index);
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

// 删除Seam
cv::Mat SeamCarver::RemoveSeam(std::vector<int> seam, cv::Mat image)
{
    cv::Mat new_img;
    new_img.create(image.rows, image.cols - 1, image.type());

    const int rows = image.rows;
    const int cols = image.cols;

    for (int i = 0; i < rows; ++i)
    {
        int temp = seam[i];
        // 对于要移除的seam，左边的像素保持不动
        for (int j = 0; j < temp; ++j)
        {
            new_img.at<cv::Vec3b>(i, j) = image.at<cv::Vec3b>(i, j);
        }
        // 右边的像素向左缩进一格
        for (int j = temp; j < cols - 1; ++j)
        {
            new_img.at<cv::Vec3b>(i, j) = image.at<cv::Vec3b>(i, j + 1);
        }
    }
    return new_img;
}

// 插入Seam
cv::Mat SeamCarver::AddSeam(std::vector<int> seam, cv::Mat image)
{
    cv::Mat new_img;
    new_img.create(image.rows, image.cols + 1, image.type());

    const int rows = image.rows;
    const int cols = image.cols;

    for (int i = 0; i < rows; ++i)
    {
        int temp = seam[i];
        // 对于要移除的seam，左边的像素保持不动
        for (int j = 0; j < temp; ++j)
        {
            new_img.at<cv::Vec3b>(i, j) = image.at<cv::Vec3b>(i, j);
        }
        // 右边的像素向右移一格
        for (int j = temp + 1; j < cols + 1; ++j)
        {
            new_img.at<cv::Vec3b>(i, j) = image.at<cv::Vec3b>(i, j - 1);
        }
        // 边界检测
        if (temp == 0)
            new_img.at<cv::Vec3b>(i, seam[i]) = new_img.at<cv::Vec3b>(i, temp + 1);
        else if (temp == cols)
            new_img.at<cv::Vec3b>(i, seam[i]) = new_img.at<cv::Vec3b>(i, temp - 1);
        // 非边界，对出来的一行做平均值处理
        else
            new_img.at<cv::Vec3b>(i, temp) =
                new_img.at<cv::Vec3b>(i, temp - 1) * 0.5f + new_img.at<cv::Vec3b>(i, temp + 1) * 0.5f;
    }
    return new_img;
}

