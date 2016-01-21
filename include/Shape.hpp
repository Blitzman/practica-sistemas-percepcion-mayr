#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXYHSL.hpp"
#include "PointXYLAB.hpp"

template <class T> class Shape
{
public:

	Shape()
	{

	}

	cv::Point getCentroid()
	{
	  std::pair<int,int> centroid;

		for(int i = 0; i < pointList.size(); i++)
    {
      centroid.first += pointList[i].x;
      centroid.second += pointList[i].y;
    }
	  centroid.first /= pointList.size();
		centroid.second /= pointList.size();

	  return cv::Point(centroid.second, centroid.first);
	}

	std::string getSemanticAverageColor()
	{
		T point_ = getAverageColor();
		return point_.getSemanticColor();
	}

	T getAverageColor()
	{
		T color_avg_;
	  for(int i = 0; i < pointList.size(); i++)
    {
      color_avg_.values[0] += pointList[i].values[0];
      color_avg_.values[1] += pointList[i].values[1];
      color_avg_.values[2] += pointList[i].values[2];
    }

		color_avg_.values[0] /= pointList.size();
	  color_avg_.values[1] /= pointList.size();
		color_avg_.values[2] /= pointList.size();

	  return color_avg_;
	}

	bool push_back(T point)
	{
    pointList.push_back(point);
	}

private:
  std::vector<T> pointList;

};




#endif
