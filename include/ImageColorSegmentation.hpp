#ifndef IMAGECOLORSEGMENTATION_HPP_
#define IMAGECOLORSEGMENTATION_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "PointXYLAB.hpp"
#include "PointXYHSL.hpp"
#include "Shape.hpp"

class ImageColorSegmentation
{

public:

  static const int ICS_IMAGE = 0;        // Image source
  static const int ICS_VIDEO = 1;        // Video source

ImageColorSegmentation(int ics_format, std::string path)
{
  this->ics_format = ics_format;
  this->path = path;

  if(ics_format == 0) // FRAME OR SINGLE IMAGE
    {
      image_ = cv::imread(path);

    } else {    // VIDEO OR IMAGESET

    vcap = cv::VideoCapture(path);
  }
}

cv::Mat processFrame()
{

  if (!image_.data)
    {
      std::cout << "No image data \n";
      exit(0);
    }

  image_.copyTo(image_color_);

  // Convert to grayscale
  cvtColor(image_, image_, cv::COLOR_BGR2GRAY );

  // Tresholding image binary inverted and otsu's method
  cv::threshold(image_, image_, 0, 255, cv::THRESH_BINARY_INV+cv::THRESH_OTSU);

  // Removing noises made by shadows/lights by opening
  cv::Mat kernel = cv::Mat::ones(cv::Size(3,3), CV_8U);
  cv::Mat opening;
  morphologyEx(image_, opening, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2);

  // Sure BG area, by applying mat morph (dilate) you assure that BG is in fact BG
  cv::dilate(opening, sure_bg_, kernel, cv::Point(1,1), 3);

  // Sure foreground area by calculing the distance to the closest zero pixel for each pixel and thresholding by the max distance
  cv::Mat dist_transform;
  distanceTransform(opening, dist_transform, CV_DIST_L2, 5);
  double min, max;
  cv::minMaxLoc(dist_transform, &min, &max);
  cv::threshold(dist_transform, sure_fg_, 0.5*max, 255, 0);

  // Geeting unknown region bu subtracting the sureFG from sureBG
  sure_fg_.convertTo(sure_fg_, CV_8U);
  subtract(sure_bg_, sure_fg_, unknown_);

  // Labelling the blobs
  cv::Mat markers;
  connectedComponents (sure_fg_, markers); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
  cv::minMaxLoc(markers, &min, &max);
  std::cout << "There are " << max << " different shapes" << std::endl;

  // Adding 1 to every label to make the background 1 instead of 0
  cv::Size tam = markers.size();
  for(int i = 0; i < tam.height; i++)
    {
      for(int j = 0; j < tam.width; j++)
        {
          markers.at<int>(i,j) = markers.at<int>(i,j) + 1;
        }
    }

  // making unknown zones to be labelled as 0
  for(int i = 0; i < tam.height; i++)
    {
      for(int j = 0; j < tam.width; j++)
        {
          if (unknown_.at<uchar>(i,j) == 255)
            markers.at<int>(i,j) = 0;
        }
    }

  // Watershed algorithm
  cv::watershed(image_color_, markers);

  std::vector<Shape<PointXY> > shapes(max);
  
  // List of shapes
  
  // Convert color image to HSV Scheme for easier color classfication
  cv::Mat color_HSV;
  cvtColor(image_color_, color_HSV, cv::COLOR_BGR2HLS);

  // Masking the image with color per BG and SHAPES
  cv::Mat color_mask;
  image_color_.copyTo(color_mask);

  // Creating the color mask by coloring shapes and BG
  for(int i = 0; i < tam.height; i++)
    {
      for(int j = 0; j < tam.width; j++)
        {
          if (markers.at<int>(i,j) == 1) // background
            {
              color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(0,255,0);

            }

          for(int l = 2; l <= max+1; l++)
            {
              if (markers.at<int>(i,j) == l) // shape
                {
                  PointXYHSL p(i,j, color_HSV.at<cv::Vec3b>(i,j)[0], color_HSV.at<cv::Vec3b>(i,j)[1], color_HSV.at<cv::Vec3b>(i,j)[2]);

                  shapes[l-2].push_back(p);

                  color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(127,0,0);
                }
            }

        }
    }

  for(int i = 0; i < max; i++) // color classification, centroid calcs and outputs
    {
      cv::Point c = shapes[i].getCentroid();
      std::string semColor = shapes[i].getSemanticAverageColor();

      std::cout << "(" << c.x << ", " << c.y << ") " << semColor << std::endl;

      circle(color_mask, c, 10, 0, -1);
      putText(color_mask, semColor, c, cv::FONT_HERSHEY_SIMPLEX, 5, cvScalar(0,0,0), 5);

    }


  return color_mask;

}

bool process(cv::Mat &frame)
{

  if(ics_format == 1) // VIDEO stream
    {

      if(vcap.read(image_))
        {
          if (!image_.data)
            {
              std::cout << "No image data \n";
              return false;
            }

        }
      else 
        return false;
    }

  frame = processFrame();
 
  return true;
}

  
private:
  int ics_format;            // Whether IMAGE or VIDEO input
  unsigned short ics_color;  // Whether HSV or LAB format color
  std::string path;          // Path to resource
  cv::Mat image_;            // Current image or frame
  cv::Mat image_color_;      // Converted to HSL
  cv::Mat sure_bg_;          // Sure Background area
  cv::Mat sure_fg_;          // Sure Foreground area
  cv::Mat unknown_;          // Unknown area between surefg and surebg

  cv::VideoCapture vcap;      // Video stream

 
};



#endif
