#include "ImageShapeSegmentation.hpp"
#include "ImageColorSegmentation.hpp"

// #include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <glob.h>
#include <unistd.h>

std::vector<std::string> globVector(const std::string& pattern){
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
		std::vector<std::string> files;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        files.push_back(std::string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return files;
}


int main(int argc, char** argv )
{
  std::vector<std::string> files = globVector("../resources/*");

  cv::Size size(640, 480);
  for(int i = 0; i < files.size(); i++)
  {
    cv::Mat frame;
    std::cout << files[i] << std::endl;
    ImageColorSegmentation ics(files[i]);
    ics.setHistogramOutput(false);
    ics.process(ImageColorSegmentation::HLS, frame);
    cv::Mat color_resized;
    resize(frame, color_resized, size);
    cv::namedWindow( "DisplayWindow" ); // Create a window for display.
    cv::imshow("DisplayWindow", color_resized );

		cv::Mat frame_shape_;
		ImageShapeSegmentation iss_(files[i]);
		iss_.process(frame_shape_);
		cv::Mat shape_resized_;
		resize(frame_shape_, shape_resized_, size);
		cv::namedWindow("DisplayWindow2");
		cv::imshow("DisplayWindow2", shape_resized_);

		cv::waitKey(0);
  }
  cv::waitKey(0); // */

	return 0;
}
