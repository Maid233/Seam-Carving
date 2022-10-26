#include "SeamCarver.h"

void SeamCarver::VerticalChangeAspectRatio(int size)
{
    for (int i = 0; i < size; ++i)
    {
        // Vertical
        CalculateEnergyMap(output_img);
        FindSeam(energy_map);

        // 实时显示找到的seam
        cv::Mat carved_image = output_img.clone();
        for (int j = 0; j < output_img.rows; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                carved_image.at<cv::Vec3b>(j, seam[j])[k] = 233;
            }
        }
        cv::imshow("Processing", carved_image);
        cv::waitKey(1);

        output_img = RemoveSeam(seam, output_img);
    }
}

void SeamCarver::HorizontalChangeAspectRatio(int size)
{
    output_img = RotateImage(output_img);

    for (int i = 0; i < size; ++i)
    {
        // Horizontal
        CalculateEnergyMap(output_img);
        FindSeam(energy_map);

        // 实时显示找到的seam
        cv::Mat carved_image = output_img.clone();
        for (int j = 0; j < output_img.rows; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                carved_image.at<cv::Vec3b>(j, seam[j])[k] = 233;
            }
        }
        carved_image = RotateBack(carved_image);
        cv::imshow("Processing", carved_image);
        cv::waitKey(1);

        output_img = RemoveSeam(seam, output_img);
    }

    output_img = RotateBack(output_img);
}

void SeamCarver::VerticalImageEnlarging(int size)
{
    cv::Mat temp_img = output_img.clone();

    for (int i = 0; i < size; ++i)
    {
        CalculateEnergyMap(output_img);
        FindSeam(energy_map);
        seam_buffer.push_back(seam);
        output_img = RemoveSeam(seam, output_img);
    }

    output_img = temp_img.clone();

    for (int i = 0; i < size; ++i)
    {
        // 实时显示找到的seam
        cv::Mat carved_image = output_img.clone();
        for (int j = 0; j < output_img.rows; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                carved_image.at<cv::Vec3b>(j, seam_buffer[i][j])[k] = 233;
            }
        }
        cv::imshow("Processing", carved_image);
        cv::waitKey(1);
        output_img = AddSeam(seam_buffer[i], output_img);
    }

    seam_buffer.clear();
}

void SeamCarver::HorizontalImageEnlarging(int size)
{
    output_img = RotateImage(output_img);

    cv::Mat temp_img = output_img.clone();

    for (int i = 0; i < size; ++i)
    {
        CalculateEnergyMap(output_img);
        FindSeam(energy_map);
        seam_buffer.push_back(seam);
        output_img = RemoveSeam(seam, output_img);
    }

    output_img = temp_img.clone();

    for (int i = 0; i < size; ++i)
    {
        // 实时显示找到的seam
        cv::Mat carved_image = output_img.clone();
        for (int j = 0; j < output_img.rows; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                carved_image.at<cv::Vec3b>(j, seam_buffer[i][j])[k] = 233;
            }
        }
        carved_image = RotateBack(carved_image);
        cv::imshow("Processing", carved_image);
        cv::waitKey(1);
        output_img = AddSeam(seam_buffer[i], output_img);
    }

    output_img = RotateBack(output_img);
    seam_buffer.clear();
}

void SeamCarver::ContentEnhancement(int size)
{
    const int cols = output_img.cols + size;
    const int rows = output_img.rows + size;
    resize(output_img, output_img, cv::Size(cols, rows));
    OptimalOrder(size);
}

void SeamCarver::OptimalOrder(int size)
{
    int vCount = 0;
    int hCount = 0;

    for (int i = 0; i < size * 2; ++i)
    {
        // Vertical
        CalculateEnergyMap(output_img);
        int e1 = FindEnergy(energy_map);

        // Horizontal
        output_img = RotateImage(output_img);
        CalculateEnergyMap(output_img);
        int e2 = FindEnergy(energy_map);
        output_img = RotateBack(output_img);

        if (e1 < e2 && vCount < size)
        {
            VerticalChangeAspectRatio(1);
            vCount++;
        }
        else if (e2 <= e1 && hCount < size)
        {
            HorizontalChangeAspectRatio(1);
            hCount++;
        }
        else
        {
            VerticalChangeAspectRatio(1);
            vCount++;
        }
    }
}

int SeamCarver::FindEnergy(const cv::Mat energy_map)
{
    const int rows = energy_map.rows;
    const int cols = energy_map.cols;

    std::vector<std::vector<int>> map_data;
    map_data.resize(rows);
    for (int i = 0; i < rows; ++i)
    {
        map_data[i].resize(cols);
    }

    int temp = 999999;
    int temp_index = -1;

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
    return energy_map.at<uchar>(rows - 1, temp_index);
}

