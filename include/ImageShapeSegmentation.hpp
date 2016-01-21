#ifndef IMAGESHAPESEGMENTATION_HPP_
#define IMAGESHAPESEGMENTATION_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "PointXY.hpp"
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

		double threshold_value_ =	cv::threshold(image_, image_, 0, 255, cv::THRESH_BINARY_INV+cv::THRESH_OTSU);

    // Removing noises made by shadows/lights by opening
		cv::Mat kernel_ = cv::Mat::ones(cv::Size(3,3), CV_8U);
		cv::Mat opening_;
		morphologyEx(image_, opening_, cv::MORPH_OPEN, kernel_, cv::Point(-1,-1), 2);

		cv::threshold(opening_, opening_, 128, 255, cv::THRESH_BINARY);

		// Sure BG area, by applying mat morph (dilate) you assure that BG is in fact BG
		/*cv::dilate(opening_, sure_bg_, kernel_, cv::Point(1,1), 3);
		// Sure foreground area by calculing the distance to the closest zero pixel for each pixel and thresholding by the max distance
		cv::Mat dist_transform_;
		distanceTransform(opening_, dist_transform_, CV_DIST_L2, 5);
		double min_, max_;
		cv::minMaxLoc(dist_transform_, &min_, &max_);
		cv::threshold(dist_transform_, sure_fg_, 0.5*max_, 255, 0);

		// Getting unknown region bu subtracting the sureFG from sureBG
		sure_fg_.convertTo(sure_fg_, CV_8U);
		subtract(sure_bg_, sure_fg_, unknown_);

		// Labelling the blobs
		cv::Mat markers_;
		connectedComponents (sure_fg_, markers_); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
		cv::minMaxLoc(markers_, &min_, &max_);
		std::cout << "There are " << max_ << " different shapes" << std::endl;

		// Adding 1 to every label to make the background 1 instead of 0
		cv::Size tam_ = markers_.size();
		for(int i = 0; i < tam_.height; i++)
		{
			for(int j = 0; j < tam_.width; j++)
			{
				markers_.at<int>(i,j) = markers_.at<int>(i,j) + 1;
			}
		}

		//  Making unknown zones to be labelled as 0
		for(int i = 0; i < tam_.height; i++)
		{
			for(int j = 0; j < tam_.width; j++)
			{
				if (unknown_.at<uchar>(i,j) == 255)
					 markers_.at<int>(i,j) = 0;
			}
		}

		// Watershed algorithm
		cv::watershed(image_color_, markers_);

	  std::vector<Shape> shapes(max_);
	  // List of shapes

	  // Convert color image to HSV Scheme for easier color classfication
		cv::Mat color_HSV;
	  cvtColor(image_color_, color_HSV, cv::COLOR_BGR2HLS);

		// Masking the image with color per BG and SHAPES
	  cv::Mat color_mask;
		image_color_.copyTo(color_mask);

	  // Creating the color mask by coloring shapes and BG
		for(int i = 0; i < tam_.height; i++)
		{
			for(int j = 0; j < tam_.width; j++)
      {
				if (markers_.at<int>(i,j) == 1) // background
        {
					color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(0,255,0);
				}

				for(int l = 2; l <= max_+1; l++)
        {
					if (markers_.at<int>(i,j) == l) // shape
					{
						PointXY p(i,j, color_HSV.at<cv::Vec3b>(i,j)[0], color_HSV.at<cv::Vec3b>(i,j)[1], color_HSV.at<cv::Vec3b>(i,j)[2]);
						shapes[l-2].add_point(p);
						color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(127,0,0);
					}
        }
      }
    }

		cv::Mat image_grayscale_ = color_mask.clone();
		cvtColor(image_grayscale_, image_grayscale_, cv::COLOR_BGR2GRAY);*/

		std::vector<std::vector<cv::Point>> contours_;
		cv::Mat contour_output_ = opening_.clone();
		cv::findContours(contour_output_, contours_, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

		std::vector<std::vector<cv::Point>> contours_poly_(contours_.size());
		std::vector<Shape> shapes_(contours_.size());

		for (unsigned int i = 0; i < contours_.size(); ++i)
		{
			cv::approxPolyDP(cv::Mat(contours_[i]), contours_poly_[i], cv::arcLength(contours_[i], true)*0.02, true);

			if (contours_poly_[i].size() < 8 || contours_poly_[i].size() == 12)
			{
				for (cv::Point p_: contours_poly_[i])
					shapes_[i].add_vertex(p_);

				shapes_[i].draw_contour(image_color_, cv::Scalar(0, 0, i * 255 / contours_.size()));
				shapes_[i].draw_name(image_color_, cv::Scalar(0, 0, i * 255 / contours_.size()));
			}
		}

		// Circle detection
		//cv::cvtColor(image_, src_gray, cv::COLOR_BGR2GRAY);
		//double otsu_value = cv::threshold( src_gray, dst2, 0, 255, 1 | CV_THRESH_OTSU );

		cv::Mat hough_input_;
		std::vector<cv::Vec3f> circles_;			
		cv::GaussianBlur(opening_, hough_input_, cv::Size(9, 9), 2, 2);
							
		cv::HoughCircles(hough_input_, circles_, cv::HOUGH_GRADIENT, 1, opening_.rows/8, (int)threshold_value_/2, (int)threshold_value_/3);//, 0, 600 );

		for(unsigned int i = 0; i < circles_.size(); ++i)
		{
			std::cout << "Circle detected\n";
			cv::Point center_(cvRound(circles_[i][0]), cvRound(circles_[i][1]));
			int radius_ = cvRound(circles_[i][2]);
			
			Shape circle_;
			circle_.set_radius(radius_);
			circle_.add_vertex(center_);
			circle_.draw_contour(image_color_, cv::Scalar(0, 0, 0));
			circle_.draw_name(image_color_, cv::Scalar(0, 0, 0));

			shapes_.push_back(circle_);
		}

		return image_color_;
	}
};

#endif
