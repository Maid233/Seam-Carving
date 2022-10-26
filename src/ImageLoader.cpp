#include "ImageLoader.h"

// 读取
int ImageLoader::LoadImage(const char *filename, cv::Mat &img)
{
    img = cv::imread(filename);
    if (img.empty())
    {
        std::cout << "ERROR: could not read the img";
        return 1;
    }
    else
    {
        std::cout << "img read.\n";
        return 0;
    }
}

// 保存
int ImageLoader::SaveImage(const char *filename, cv::Mat &img)
{
    cv::imwrite(filename, img);
    if (img.empty())
    {
        std::cout << "ERROR: could not save the img";
        return 1;
    }
    else
    {
        std::cout << "img saved.\n";
        return 0;
    }
}