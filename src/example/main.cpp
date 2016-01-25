#include "ImageShapeSegmentation.hpp"

#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv)
{
	ImageShapeSegmentation iss_(1, "../resources/Video_2/output-%04d.jpg");
	//ImageShapeSegmentation iss_(0, "../../../resources/Picture_21.jpg");
	//ImageShapeSegmentation iss_(0, "../resources/Video_2/output-0047.jpg");

	cv::Mat frame_;
	cv::Size size_(640, 480);
	iss_.process(frame_);

	while (iss_.process(frame_))
	{
		std::cout << "Processing frame...\n";
	    cv::Mat color_resized_;
	    resize(frame_, color_resized_, size_);
	    cv::namedWindow("ShapeSegmentation", cv::WINDOW_NORMAL);
	    cv::imshow("ShapeSegmentation", color_resized_);
			cv::waitKey(0);
	}

  cv::waitKey(0);

	return 0;
}
