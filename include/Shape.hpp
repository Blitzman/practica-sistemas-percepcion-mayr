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

  Shape()
	{
		m_radius = -1;
  }

  cv::Point getCentroid()
  {
    std::pair<int,int> centroid;

    for(unsigned short i = 0; i < m_point_list.size(); ++i)
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
		if (v_ == 3)
			shape_name_ = "triangle";
		else if (v_ == 4)
			shape_name_ = "square";
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

   std::string getSemanticAverageColorLAB()
  {
    PointXY point_ = getAverageColor();
    return point_.getSemanticColorLAB();
  }


  PointXY getAverageColor()
  {
    PointXY color_avg_;

    for(unsigned short i = 0; i < m_point_list.size(); ++i)
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
		if (m_radius < 0 && m_vertices.size() <= 6)
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
		}

		// A circle
		if (m_radius > 0)
		{
			m_area = (double)M_PI * pow(m_radius, 2);
		}
	}

  void draw_contour (cv::Mat & rImage, const cv::Scalar & crColor)
  {
		if (m_radius > 0.0)
		{
			cv::circle(rImage, m_vertices[0], 3, crColor, -1, 8, 0 );
			cv::circle(rImage, m_vertices[0], m_radius, crColor, 3, 8, 0);
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

	void draw_name (cv::Mat & rImage, const cv::Scalar & crColor)
	{

		std::ostringstream area_;
		area_ << std::fixed << std::setprecision(1) << m_area;

		cv::putText(rImage, get_semantic_shape() + ":" + area_.str(),//std::to_string(m_area),
				get_vertex_centroid(), cv::FONT_HERSHEY_SIMPLEX, 3, crColor, 5);
	}

private:
  std::vector<PointXY> m_point_list;
  std::vector<cv::Point> m_vertices;
	int m_radius;
	double m_area;
};

#endif
