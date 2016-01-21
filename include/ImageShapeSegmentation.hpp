#ifndef IMAGESHAPESEGMENTATION_HPP_
#define IMAGESHAPESEGMENTATION_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "PointXYHSL.hpp"
#include "Shape.hpp"

class ImageShapeSegmentation
{
public:

	static const int ICS_IMAGE = 0;
	static const int ICS_VIDEO = 1;

	ImageShapeSegmentation(const int & crIcsFormat, const std::string & crPath)
	{
		m_ics_format = crIcsFormat;
		m_path = crPath;

    if(m_ics_format == 0)
    {
			m_image = cv::imread(m_path);
    } 
		else
		{
			m_vcap = cv::VideoCapture(m_path);
    }
	}

	bool process(cv::Mat & rFrame)
	{
    if(m_ics_format == 1)
    {
			if(m_vcap.read(m_image))
      {
				if (!m_image.data)
        {
					std::cerr << "No image data \n";
          return false;
        }
      }
      else 
			{
				std::cerr << "No image read...\n";
				return false;
			}
    }

    rFrame = processFrame();
 
    return true;
	}

private:

	int m_ics_format;
	std::string m_path;
	cv::Mat m_image;
	cv::VideoCapture m_vcap;

	cv::Mat processFrame()
	{
    if (!m_image.data)
    {
        std::cerr << "No image data \n";
        exit(0);
    }

		cv::Mat image_;
		cv::Mat image_color_;
		cv::Mat sure_bg_;
		cv::Mat sure_fg_;
		cv::Mat unknown_;

		m_image.copyTo(image_);
		m_image.copyTo(image_color_);
		cvtColor(image_, image_, cv::COLOR_BGR2GRAY);

		cv::threshold(image_, image_, 0, 255, cv::THRESH_BINARY_INV+cv::THRESH_OTSU);

    // Removing noises made by shadows/lights by opening
		cv::Mat kernel_ = cv::Mat::ones(cv::Size(3,3), CV_8U);
		cv::Mat opening_;
		morphologyEx(image_, opening_, cv::MORPH_OPEN, kernel_, cv::Point(-1,-1), 2);

		cv::threshold(opening_, opening_, 128, 255, cv::THRESH_BINARY);

		std::vector<std::vector<cv::Point>> contours_;
		cv::Mat contour_output_ = opening_.clone();
		cv::findContours(contour_output_, contours_, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		std::vector<std::vector<cv::Point>> contours_poly_(contours_.size());
		for (unsigned int i = 0; i < contours_.size(); ++i)
		{
			cv::approxPolyDP(cv::Mat(contours_[i]), contours_poly_[i], cv::arcLength(contours_[i], true)*0.02, true);
			
			if (contours_poly_[i].size() == 3)
			{
				std::cout << "Detected triangle...\n";
			}

			for (unsigned int j = 0; j < contours_poly_[i].size()-1; ++j)
			{
				cv::line(image_color_, contours_poly_[i][j], contours_poly_[i][j+1], cv::Scalar(0,0,i*255/contours_.size()), 10);
			}
			
			if (contours_poly_[i].size() > 2)
				cv::line(image_color_, contours_poly_[i][contours_poly_[i].size()-1], contours_poly_[i][0], cv::Scalar(0,0,i*255/contours_.size()), 10);
		}

		// Sure BG area, by applying mat morph (dilate) you assure that BG is in fact BG
		/*cv::dilate(opening, sure_bg_, kernel, cv::Point(1,1), 3);
		// Sure foreground area by calculing the distance to the closest zero pixel for each pixel and thresholding by the max distance
		cv::Mat dist_transform;
		distanceTransform(opening, dist_transform, CV_DIST_L2, 5);
		double min, max;
		cv::minMaxLoc(dist_transform, &min, &max);
		cv::threshold(dist_transform, sure_fg_, 0.5*max, 255, 0);

		// Getting unknown region bu subtracting the sureFG from sureBG
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

		//  Making unknown zones to be labelled as 0
		for(int i = 0; i < tam.height; i++)
		{
			for(int j = 0; j < tam.width; j++)
			{
				if (unknown_.at<uchar>(i,j) == 255)
					 markers.at<int>(i,j) = 0;
			}
		}

		// Watershed algorithm
		cv::watershed(image_color_, markers);*/

    return image_color_;
	}
};

#endif
