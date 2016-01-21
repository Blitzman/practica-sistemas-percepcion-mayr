#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class Shape
{
public:

  Shape()
  {

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

  std::string getSemanticAverageColor()
  {
    PointXY point_ = getAverageColor();
    return point_.getSemanticColor();
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

  void add_point(PointXY point)
  {
    m_point_list.push_back(point);
  }

  void add_vertex (const cv::Point & crVertex)
  {
    m_vertices.push_back(crVertex);
  }

  void draw_contour (cv::Mat & rImage, const cv::Scalar & crColor)
  {
    for (unsigned int i = 0; i < m_vertices.size()-1; ++i)
      {
        cv::line(rImage, m_vertices[i], m_vertices[i+1], crColor, 10);
      }

    if (m_vertices.size() > 2)
      cv::line(rImage, m_vertices[m_vertices.size()-1], m_vertices[0], crColor, 10);
  }

private:
  std::vector<PointXY> m_point_list;
  std::vector<cv::Point> m_vertices;

};

#endif
