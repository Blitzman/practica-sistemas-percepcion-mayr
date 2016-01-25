#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class Shape
{
public:

  std::vector<PointXY> m_point_list;

  Shape()
	{
		m_radius = -1;
		m_isEllipse = false;
  }

	Shape(std::vector<PointXY> points)
	{
		m_point_list = points;
		m_radius = -1;
		m_isEllipse = false;
	}

  cv::Point getCentroid()
  {
    std::pair<int,int> centroid;

    for(unsigned int i = 0; i < m_point_list.size(); ++i)
    {
			centroid.first += m_point_list[i].x;
			centroid.second += m_point_list[i].y;
    }

    centroid.first /= m_point_list.size();
    centroid.second /= m_point_list.size();

    return cv::Point(centroid.second, centroid.first);
  }

	cv::Point get_vertex_centroid()
	{
		cv::Point centroid_;
		centroid_.x = 0;
		centroid_.y = 0;

		for (cv::Point p_: m_vertices)
		{
			centroid_.x += p_.x;
			centroid_.y += p_.y;
		}

		centroid_.x /= m_vertices.size();
		centroid_.y /= m_vertices.size();

		return centroid_;
	}

	std::string get_semantic_shape()
	{
		std::string shape_name_ = "unk";
		unsigned int v_ = m_vertices.size();

		if (m_radius > 0)
			shape_name_ = "circle";
		if (m_radius < 0 && m_isEllipse)
			shape_name_ = "ellipse";
		if (v_ == 3)
			shape_name_ = "triangle";
		else if (v_ == 4 && isSquare() == 2)
			shape_name_ = "square";
		else if (v_ == 4 && isSquare() == 3)
			shape_name_ = "rhombus";
		else if (v_ == 4 && isSquare() == 1)
			shape_name_ = "rectangle";
		else if (v_ == 5)
			shape_name_ = "pentagon";
		else if (v_ == 6)
			shape_name_ = "hexagon";
		else if (v_ == 12)
			shape_name_ = "star";
		
		return shape_name_;
	}

  std::string getSemanticAverageColorHSV()
  {
    PointXY point_ = getAverageColor();
    return point_.getSemanticColorHSV();
  }

	std::string getSemanticAverageColorHLS()
	{
		PointXY p_ = getAverageColor();
    float hue = p_.values[0];
    float sat = p_.values[2];
    float lgt = p_.values[1];

    //if (lgt < 0.2)  return "Black";
    //if (lgt > 0.8)  return "Whites";
    //if (sat < 0.25) return "Gray";

    if (hue < 40)   return "Red";
    //if (hue < 90)   return "Yellows";
    if (hue < 80)  return "Green";
    //if (hue < 210)  return "Cyan";
    if (hue < 150)  return "Blue";
    if (hue < 255)  return "Magent";
	}

  std::string getSemanticAverageColorLAB()
  {
    PointXY point_ = getAverageColor();
    return point_.getSemanticColorLAB();
  }

  PointXY getAverageColor()
  {
    PointXY color_avg_;

    for(unsigned int i = 0; i < m_point_list.size(); ++i)
    {
      color_avg_.values[0] += m_point_list[i].values[0];
      color_avg_.values[1] += m_point_list[i].values[1];
			color_avg_.values[2] += m_point_list[i].values[2];
    }

    color_avg_.values[0] /= m_point_list.size();
    color_avg_.values[1] /= m_point_list.size();
    color_avg_.values[2] /= m_point_list.size();

    return color_avg_;
  }

	void set_radius(const int & crRadius)
	{
		m_radius = crRadius;
	}

  void add_point(PointXY point)
  {
    m_point_list.push_back(point);
  }

  void add_vertex (const cv::Point & crVertex)
  {
    m_vertices.push_back(crVertex);
  }

	void postprocess ()
	{

		// Not a circle
		if (m_radius < 0 )//&& (m_vertices.size() <= 6 || m_vertices.size() == 12))
		{
			float avg_side_ = 0.0f;
			for (unsigned int i = 0; i < m_vertices.size(); ++i)
			{
				unsigned int j = (i == m_vertices.size() - 1) ? 0 : i+1;
				avg_side_ = std::max((float)cv::norm(m_vertices[i]-m_vertices[j]), avg_side_);
			}

			for (unsigned int i = 0; i < m_vertices.size(); ++i)
			{
				unsigned int j = (i == m_vertices.size() - 1) ? 0 : i+1;
				float side_ = cv::norm(m_vertices[i]-m_vertices[j]);

				if (std::abs(avg_side_ - side_) > avg_side_ * 0.5f)
				{
					m_vertices.erase(m_vertices.begin()+i);
					i--;
				}
			}

			m_area = cv::contourArea(m_vertices);

			// BoundingBox
			m_bounding_box = cv::minAreaRect(m_vertices);
		}

		// A circle
		if (m_radius > 0)
		{
			m_area = (double)M_PI * pow(m_radius, 2);

			// BoundingBox
			std::vector<cv::Point> points_;
			cv::Point center_ = m_vertices[0];

			for (unsigned int i = 0; i < 4; ++i)
				points_.push_back(center_);

			points_[0].x = points_[0].x - m_radius;
			points_[0].y = points_[0].y - m_radius;
			points_[1].x = points_[1].x - m_radius;
			points_[1].y = points_[1].y + m_radius;
			points_[2].x = points_[2].x + m_radius;
			points_[2].y = points_[2].y + m_radius;
			points_[3].x = points_[3].x + m_radius;
			points_[3].y = points_[3].y - m_radius;


			m_bounding_box = cv::minAreaRect(points_);
		}
	}

  void draw_contour (cv::Mat & rImage, const cv::Scalar & crColor)
  {
		if (m_radius > 0.0)
		{
			cv::circle(rImage, m_vertices[0], 3, crColor, -1, 8, 0 );
			cv::circle(rImage, m_vertices[0], m_radius, crColor, 10, 8, 0);
		}
		else if (m_isEllipse)
		{
			cv::ellipse(rImage, m_bounding_box, crColor, 10, CV_AA);
			cv::circle(rImage, m_bounding_box.center, 3, crColor, -1, 8, 0);
        	//cv::ellipse(rImage, m_bounding_box.center, m_bounding_box.size*0.5f, m_bounding_box.angle, 0, 360, crColor, 1, CV_AA);
		}
		else
		{
			for (unsigned int i = 0; i < m_vertices.size()-1; ++i)
      		{
        		cv::line(rImage, m_vertices[i], m_vertices[i+1], crColor, 10);
      		}

			if (m_vertices.size() > 2)
				cv::line(rImage, m_vertices[m_vertices.size()-1], m_vertices[0], crColor, 10);

			cv::circle(rImage, get_vertex_centroid(), 3, crColor, -1, 8, 0);
		}
  }

	void draw_box (cv::Mat & rImage, const cv::Scalar & crColor)
	{
		cv::Point2f rect_vertex_[4];

		m_bounding_box.points(rect_vertex_);

		for (unsigned int i = 0; i < 4; ++i)
		{
			cv::line(rImage, rect_vertex_[i], rect_vertex_[(i+1)%4], crColor, 3);
		}
	}

	void draw_name (cv::Mat & rImage, const cv::Scalar & crColor)
	{

		std::ostringstream area_;
		area_ << std::fixed << std::setprecision(1) << m_area;

		cv::putText(rImage, getSemanticAverageColorHSV() + " " + get_semantic_shape() + ":" + area_.str(),//std::to_string(m_area),
				get_vertex_centroid(), cv::FONT_HERSHEY_SIMPLEX, 3, crColor, 5);

		std::cout << getSemanticAverageColorHSV() << " " << get_semantic_shape() << ": " << m_area << std::endl;
		std::cout << " at " << get_vertex_centroid().x << "," << get_vertex_centroid().y << "\n";
	}

	void draw_name (cv::Mat & rImage, const cv::Scalar & crColor, const int & id_)
	{

		std::ostringstream area_;
		area_ << std::fixed << std::setprecision(1) << m_area;

		cv::putText(rImage, std::to_string(id_) + " " + getSemanticAverageColorHSV() + " " + get_semantic_shape() + ":" + area_.str(),//std::to_string(m_area),
				get_vertex_centroid(), cv::FONT_HERSHEY_SIMPLEX, 3, crColor, 5);

		std::cout << getSemanticAverageColorHSV() << " " << get_semantic_shape() << ": " << m_area << std::endl;
		std::cout << " at " << get_vertex_centroid().x << "," << get_vertex_centroid().y << "\n";
	}

	// 1 -> todos los angulos 90
	// 2 -> angulos 90 y lados iguales
	// 3 -> lados iguales
	// 0 -> nada de lo anterior
	int isSquare ()
	{
		if (m_vertices.size() == 4)
		{
			cv::Point point_, u_, v_;
			double angle_ = 0;
			std::vector<double> size_;
			double std_dev_ = 0;

			for (unsigned int i = 0; i < m_vertices.size(); ++i)
			{
				point_ = m_vertices[i];
				u_ = m_vertices[i] - m_vertices[(i + 3) % m_vertices.size()];
				v_ = m_vertices[i] - m_vertices[(i + 1) % m_vertices.size()];

				angle_ += acos(abs(u_.x * v_.x + u_.y * v_.y) / (sqrt(pow(u_.x,2) + pow(u_.y,2)) * sqrt(pow(v_.x,2) + pow(v_.y,2)))) * 180.0 / M_PI;
			}


			for (unsigned int i = 0; i < 4; ++i)
			{

				size_.push_back(cv::norm(m_vertices[(i + 1) % m_vertices.size()] - m_vertices[i]));
				//size_[i] = sqrt(
				//		pow(m_vertices[(i + 1) % m_vertices.size()].x - m_vertices[i].x, 2) +
				//		pow(m_vertices[(i + 1) % m_vertices.size()].y - m_vertices[i].y, 2));
			}


			std_dev_ = standardDeviation(size_);


			if ((abs((angle_ / m_vertices.size()) - 90)  <= 5.0) && std_dev_ <= 20.0)
				return 2;

			if (abs((angle_ / m_vertices.size()) - 90) <= 5.0)
				return 1;

			if (std_dev_ <= 20.0)
				return 3;
		}

		return 0;
	}

	bool pointWithinPolygon(const cv::Point & crPoint)
	{
		
		cv::Point2f rect_vertex_[4];

		m_bounding_box.points(rect_vertex_);

		cv::Point min_, max_;

		min_.x = rect_vertex_[0].x;
		max_.x = rect_vertex_[0].x;
		min_.y = rect_vertex_[0].y;
		max_.y = rect_vertex_[0].y;

		for (unsigned int i = 0; i < 4; ++i)
		{
			if(rect_vertex_[i].x < min_.x)
				min_.x = rect_vertex_[i].x;

			if(rect_vertex_[i].x > max_.x)
				max_.x = rect_vertex_[i].x;

			if(rect_vertex_[i].y < min_.y)
				min_.y = rect_vertex_[i].y;

			if(rect_vertex_[i].y > max_.y)
				max_.y = rect_vertex_[i].y;
		}


		if ((crPoint.x > min_.x) && (crPoint.x < max_.x) &&
			(crPoint.y > min_.y && crPoint.y < max_.y))
				return true;
		
		return false;
	}

	void convertToEllipse()
	{
		m_bounding_box = cv::fitEllipse(m_vertices);
		m_isEllipse = true;
	}

	double standardDeviation(const std::vector<double> & crdata_)
	{

		double sum_ = 0, 
				deviation_ = 0, 
				mean_ = 0;

		for(unsigned int i = 0; i < crdata_.size(); ++i)
		{
			mean_ += crdata_[i];
		}

		mean_ = mean_ / crdata_.size();


		for(unsigned int i = 0; i < crdata_.size(); ++i)
			sum_ += (double(mean_ - crdata_[i])) * (double(mean_ - crdata_[i]));

		deviation_ = sqrt(double(sum_ / crdata_.size() - 1)) ;

	
		return deviation_;
	}

	int getVertexCount() { return m_vertices.size(); }

private:

	cv::RotatedRect m_bounding_box;
  std::vector<cv::Point> m_vertices;
	int m_radius;
	double m_area;
	bool m_isEllipse;
};

#endif
