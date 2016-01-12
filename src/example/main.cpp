#include "header_test.hpp"

#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv )
{
	cv::Mat image_;
	image_ = cv::imread("lena.jpg", 1);

	foo();

	if (!image_.data)
	{
		std::cout << "No image data \n";
		return -1;
	}

	cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
	cv::imshow("Display Image", image_);

	cv::waitKey(0);

	return 0;
}
