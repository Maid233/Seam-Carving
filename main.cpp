#include <iostream>
#include <cmath>
#include <memory>
#include <opencv2/core/utils/logger.hpp>

#include "header_config.h"

#include "ObjectRemoval.h"
#include "MultiSize.h"

bool ObjectRemoval::isDrawing = false;
cv::Point ObjectRemoval::currentPoint = cv::Point(0, 0);

int main()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);

	std::string file_name = "", default_file = "test_image/test2.jpg";

	std::cout << "Input the path of the image:";
	std::getline(std::cin, file_name);

	std::string file_path = "./";
	if (file_name.length() > 0)
	{
		file_path += file_name;
	}
	else
	{
		file_path += default_file;
	}

	std::string src_dir(SRC_DIR);
	file_path = src_dir + file_path;

	int mod;
	std::cout << "Please input the mod: " << std::endl;
	std::cout << "0. Vertical Aspect Ratio Change" << std::endl;
	std::cout << "1. Horizontal Aspect Ratio Change" << std::endl;
	std::cout << "2. Vertical Image Enlarging" << std::endl;
	std::cout << "3. Horizontal Image Enlarging" << std::endl;
	std::cout << "4. Content Enhancement" << std::endl;
	std::cout << "5. Object Removal" << std::endl;
	std::cout << "6. Multi Size Image" << std::endl;
	std::cin >> mod;

	if(mod == OBJECT_REMOVAL)
	{
		ObjectRemoval sc(file_path.c_str());
	}
	else if(mod == MULTI_SIZE_IMAGE)
	{
		MultiSize sc(file_path.c_str());
	}
	else
	{
		SeamCarver sc(file_path.c_str());
		
		int size;
		std::cout << "Input the size: " << std::endl;
		std::cin >> size;

		switch (mod)
		{
			case VERTICAL_CARVE:
				sc.VerticalChangeAspectRatio(size);
				break;

			case HORIZONTAL_CARVE:
				sc.HorizontalChangeAspectRatio(size);
				break;

			case VERTICAL_ENLARGE:
				sc.VerticalImageEnlarging(size);
				break;

			case HORIZONTAL_ENLARGE:
				sc.HorizontalImageEnlarging(size);
				break;

			case CONTENT_ENHANCEMENT:
				sc.ContentEnhancement(size);
				break;
			default:
				break;
		}
		sc.Show();
	}
}
