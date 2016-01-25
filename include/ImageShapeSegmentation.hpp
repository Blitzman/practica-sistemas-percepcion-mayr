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

	ImageShapeSegmentation(const std::string & crPath)
	{
		m_path = crPath;
		m_image = cv::imread(m_path);
	}

	bool process(cv::Mat & rFrame)
	{
    rFrame = processFrame();
 
    return true;
	}

private:

	std::string m_path;
	cv::Mat m_image;

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

		// Edge sharpening with unsharp masking
		// We use a gaussian smoothing filter and subtract the smoothed version from the
		// original image in a weighted way so the values of a constant area remain constant
		cv::Mat blurred_;
		cv::GaussianBlur(image_, blurred_, cv::Size(0,0), 3);
		cv::addWeighted(blurred_, 1.5, image_, -0.5, 0, image_);

		// Convert image to grayscale
		cvtColor(image_, image_, cv::COLOR_BGR2GRAY);
		
		// Binary thresholding + OTSU
		double threshold_value_ =	cv::threshold(image_, image_, 0, 255, cv::THRESH_BINARY_INV+cv::THRESH_OTSU);

    	// Removing noises made by shadows/lights by opening and closing
		cv::Mat kernel_ = cv::Mat::ones(cv::Size(3,3), CV_8U);
		cv::Mat opening_;
		morphologyEx(image_, opening_, cv::MORPH_OPEN, kernel_, cv::Point(-1,-1), 2);
		morphologyEx(opening_, opening_, cv::MORPH_CLOSE, kernel_, cv::Point(-1,-1), 2);
		cv::threshold(opening_, opening_, 128, 255, cv::THRESH_BINARY);


		

		// Find contours
		std::vector<std::vector<cv::Point>> contours_;
		cv::Mat contour_output_ = opening_.clone();
		cv::findContours(contour_output_, contours_, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

		std::vector<std::vector<cv::Point>> contours_poly_(contours_.size());
		std::vector<Shape> shapes_(contours_.size());

		for (unsigned int i = 0; i < contours_.size(); ++i)
		{
			double precision_factor_ = 0.02;
			double precision_ = cv::arcLength(contours_[i], true) * precision_factor_;
			cv::approxPolyDP(cv::Mat(contours_[i]), contours_poly_[i], precision_, true);

			//if (contours_poly_[i].size() < 8 || contours_poly_[i].size() == 12)
			//{
				for (cv::Point p_: contours_poly_[i])
					shapes_[i].add_vertex(p_);

				shapes_[i].postprocess();

				//shapes_[i].draw_contour(image_color_, cv::Scalar(0, 0, 50 + i * 205 / contours_.size()));
				//shapes_[i].draw_name(image_color_, cv::Scalar(0, 0, 50 + i * 205 / contours_.size()));
			//}

		}


		// Circle detection
		cv::Mat hough_input_;
		std::vector<cv::Vec3f> circles_;			
		cv::GaussianBlur(opening_, hough_input_, cv::Size(9, 9), 5, 5);
							
		cv::HoughCircles(hough_input_, circles_, CV_HOUGH_GRADIENT, 1, opening_.rows/8, (int)threshold_value_/2, (int)threshold_value_/3);//, 0, 600 );

		for (unsigned int i = 0; i < circles_.size(); ++i)
		{
			std::cout << "Circle detected\n";
			cv::Point center_(cvRound(circles_[i][0]), cvRound(circles_[i][1]));
			int radius_ = cvRound(circles_[i][2]);

			for (unsigned int j = 0; j < shapes_.size(); ++j)
			{
				std::cout << shapes_[j].get_semantic_shape() << std::endl;
				if (shapes_[j].pointWithinPolygon(center_) && (shapes_[j].getVertexCount() >= 8 && shapes_[j].getVertexCount() != 12)) // The circle replaces the shape
					shapes_.erase(shapes_.begin() + j);

				//else if (shapes_[j].getVertexCount() >= 8 && shapes_[j].getVertexCount() != 12) // Convert to ellipse
				//	shapes_[j].convertToEllipse();
			}
			
			Shape circle_;
			circle_.set_radius(radius_);
			circle_.add_vertex(center_);

			circle_.postprocess();

			shapes_.push_back(circle_);
		}


		//Ellipses
		for (unsigned int i = 0; i < shapes_.size(); ++i)
		{
			if (shapes_[i].getVertexCount() >= 8 && shapes_[i].getVertexCount() != 12)
			{
				shapes_[i].convertToEllipse();
			}
		}



		//Drawing
		for (unsigned int i = 0; i < shapes_.size(); ++i)
		{
			shapes_[i].draw_contour(image_color_, cv::Scalar(0, 0, 50 + i * 205 / contours_.size()));
			shapes_[i].draw_name(image_color_, cv::Scalar(0, 0, 50 + i * 205 / contours_.size()));
			shapes_[i].draw_box(image_color_, cv::Scalar(0, 0, 0));
		}
		

		return image_color_;
	}
};

#endif
