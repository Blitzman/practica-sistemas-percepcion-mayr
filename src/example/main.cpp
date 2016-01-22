#include "ImageColorSegmentation.hpp"

#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <iostream>

using namespace std;


int main(int argc, char** argv )
{
  /*
    Example of color segmentation for a video source 
  */
  ImageColorSegmentation ics(ImageColorSegmentation::ICS_VIDEO,ImageColorSegmentation::ICS_LAB,"../resources/Video_1/output-%04d.jpg");

  cv::Mat frame;
  cv::Size size(640, 480);
  ics.process(frame);
  while (ics.process(frame))
    {
      cv::Mat color_resized;
      resize(frame, color_resized, size);
      cv::namedWindow("ColorSegmentation", cv::WINDOW_NORMAL);
      cv::imshow("ColorSegmentation", color_resized);
     
    }
  cv::waitKey(0);

  

  /*
    Example of color segmentation for a still frame or image input
    ImageColorSegmentation ics(ImageColorSegmentation::ICS_IMAGE, "../resources/Picture_17.jpg");
    cv::Mat frame;
    ics.process(frame);
    // Show frame

    */


  return 0;
}
