#ifndef IMAGECOLORSEGMENTATION_HPP_
#define IMAGECOLORSEGMENTATION_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "Shape.hpp"

class ImageColorSegmentation
{

public:

  static const int ICS_IMAGE = 0;           // Image source
  static const int ICS_VIDEO = 1;           // Video source
  const static unsigned short ICS_HSV = 0;  // Format Color
  const static unsigned short ICS_LAB = 1;  // Format Color
  

  ImageColorSegmentation(int ics_format, unsigned short ics_color, std::string path)
  {
    this->ics_format = ics_format;
    this->path = path;
    this->ics_color = ics_color;
    
    if(ics_format == 0) // FRAME OR SINGLE IMAGE
      {
        image_ = cv::imread(path);

      } else {    // VIDEO OR IMAGESET

      vcap = cv::VideoCapture(path);
    }
  }

  void preprocessingFrame(cv::Mat& rimage,double & rmin, double &rmax)
  {
    // Convert to grayscale
    cvtColor(rimage, rimage, cv::COLOR_BGR2GRAY );

    // Tresholding image binary inverted and otsu's method
    cv::threshold(rimage, rimage, 0, 255, cv::THRESH_BINARY_INV+cv::THRESH_OTSU);

    // Removing noises made by shadows/lights by opening
    cv::Mat kernel = cv::Mat::ones(cv::Size(3,3), CV_8U);
    cv::Mat opening;
    morphologyEx(rimage, opening, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2);

    // Sure BG area, by applying mat morph (dilate) you assure that BG is in fact BG
    cv::dilate(opening, sure_bg_, kernel, cv::Point(1,1), 3);

    // Sure foreground area by calculing the distance to the closest zero pixel for each pixel and thresholding by the max distance
    cv::Mat dist_transform;
    distanceTransform(opening, dist_transform, CV_DIST_L2, 5);
    double min, max;
    cv::minMaxLoc(dist_transform, &min, &max);
    cv::threshold(dist_transform, sure_fg_, 0.5*max, 255, 0);
    rmin = min;
    rmax = max;
    /*cv::Mat color_resized;
    resize(dist_transform, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorPreprocessing", cv::WINDOW_NORMAL);
    cv::imshow("ColorPreprocessing", color_resized);*/
    // Geeting unknown region bu subtracting the sureFG from sureBG
    sure_fg_.convertTo(sure_fg_, CV_8U);
    subtract(sure_bg_, sure_fg_, unknown_);

  }
  
  cv::Mat processFrame()
  {

    if (!image_.data)
      {
        std::cout << "No image data \n";
        exit(0);
      }

    image_.copyTo(image_color_);
    double min = 0.0,max = 0.0;
    
    preprocessingFrame(image_,min,max);
    
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

    // List of shapes
    std::vector<Shape> shapes(max);

    cv::Mat color_mask;
    //doHSV(markers,max,shapes,color_mask);
    doLAB(markers,max,shapes,color_mask);
    

    return color_mask;

  }

  void doLAB(const cv::Mat& markers, double max, std::vector<Shape> shapes, cv::Mat& color_mask)
  {
    cv::Size tam = markers.size();
    cv::Mat color_LAB;
    cv::Mat color_resized;
    /*resize(image_color_, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorHSVPrueba", cv::WINDOW_NORMAL);
    cv::imshow("ColorHSVPrueba", color_resized);*/
    // Convert color image to HSV Scheme for easier color classfication
   
    cvtColor(image_color_, color_LAB, cv::COLOR_BGR2Lab);
    //cv::Mat color_resized;
    resize(color_LAB, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorLAB", cv::WINDOW_NORMAL);
    cv::imshow("ColorLAB", color_resized);
    
    // Masking the image with color per BG and SHAPES
    image_color_.copyTo(color_mask);

    /*PROBEEE! for(int row = 0; row < color_LAB.rows;row++)
      {
        const uchar * ptr = (uchar *)color_LAB.data ;//+ row*color_LAB.)
        for(int col = 0; col < color_LAB.cols;col++)
          {
            std::cout<<"int = " <<std::endl <<" " <<(int)* ptr <<std::endl<<std::endl;
            
          }

      }
    */
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
                    PointXY p(i,j, color_LAB.at<cv::Vec3b>(i,j)[0], color_LAB.at<cv::Vec3b>(i,j)[1], color_LAB.at<cv::Vec3b>(i,j)[2]);

                    shapes[l-2].add_point(p);

                    color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(127,0,0);
                  }
              }

          }
      }
    resize(color_mask, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorMask", cv::WINDOW_NORMAL);
    cv::imshow("ColorMask", color_resized);
    for(int i = 0; i < max; i++) // color classification, centroid calcs and outputs
      {
        cv::Point c = shapes[i].getCentroid();
        std::string semColor = shapes[i].getSemanticAverageColorLAB();

        std::cout << "(" << c.x << ", " << c.y << ") " << semColor << std::endl;

        circle(color_mask, c, 10, 0, -1);
        putText(color_mask, semColor, c, cv::FONT_HERSHEY_SIMPLEX, 5, cvScalar(0,0,0), 5);
      }
  }

  void doHSV(const cv::Mat& markers,double max,std::vector<Shape> shapes,cv::Mat& color_mask)
  {
    cv::Size tam = markers.size();
    cv::Mat color_HSV;
    cv::Mat color_resized;
    /*resize(image_color_, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorHSVPrueba", cv::WINDOW_NORMAL);
    cv::imshow("ColorHSVPrueba", color_resized);*/
    // Convert color image to HSV Scheme for easier color classfication
   
    cvtColor(image_color_, color_HSV, cv::COLOR_BGR2HLS);
    //cv::Mat color_resized;
    /*resize(color_HSV, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorHSV", cv::WINDOW_NORMAL);
    cv::imshow("ColorHSV", color_resized);*/
    
    // Masking the image with color per BG and SHAPES
    
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
                    PointXY p(i,j, color_HSV.at<cv::Vec3b>(i,j)[0], color_HSV.at<cv::Vec3b>(i,j)[1], color_HSV.at<cv::Vec3b>(i,j)[2]);

                    shapes[l-2].add_point(p);

                    color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(127,0,0);
                  }
              }

          }
      }
    /*resize(color_mask, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorMask", cv::WINDOW_NORMAL);
    cv::imshow("ColorMask", color_resized);*/
    for(int i = 0; i < max; i++) // color classification, centroid calcs and outputs
      {
        cv::Point c = shapes[i].getCentroid();
        std::string semColor = shapes[i].getSemanticAverageColorHSV();

        std::cout << "(" << c.x << ", " << c.y << ") " << semColor << std::endl;

        circle(color_mask, c, 10, 0, -1);
        putText(color_mask, semColor, c, cv::FONT_HERSHEY_SIMPLEX, 5, cvScalar(0,0,0), 5);

      }
    //return color_mask;
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
