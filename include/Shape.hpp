#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXYHSL.hpp"

class Shape
{
	private:
		// std::vector<PointXYHSL> pointList;

	public:
		//Shape(int t);
		Shape();
		Shape(std::vector<PointXYHSL>);
		cv::Point getCentroid();
		cv::Vec3i getAverageColor();
		bool push_back(PointXYHSL);
		std::string getSemanticAverageColor();

        std::vector<PointXYHSL> pointList;


};

Shape::Shape()
{

}

Shape::Shape(std::vector<PointXYHSL> v)
{
    pointList = v;
}

cv::Point Shape::getCentroid()
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

std::string Shape::getSemanticAverageColor()
{
	cv::Vec3i c = getAverageColor();
    float hue = c[0];
    float sat = c[2];
    float lgt = c[1];

    //if (lgt < 0.2)  return "Black";
    //if (lgt > 0.8)  return "Whites";
    //if (sat < 0.25) return "Gray";

    if (hue < 40)   return "Red";
    //if (hue < 90)   return "Yellows";
    if (hue < 80)  return "Green";
    //if (hue < 210)  return "Cyan";
    if (hue < 150)  return "Blue";
    if (hue < 255)  return "Magent";

    return "Red";
}

cv::Vec3i Shape::getAverageColor()
{
	cv::Vec3i colorAvg;
	for(int i = 0; i < pointList.size(); i++)
	{
		colorAvg[0] += pointList[i].h;
        colorAvg[1] += pointList[i].s;
        colorAvg[2] += pointList[i].v;
	}
	colorAvg[0] /= pointList.size();
	colorAvg[1] /= pointList.size();
	colorAvg[2] /= pointList.size();

	return colorAvg;
}

bool Shape::push_back(PointXYHSL point)
{
	pointList.push_back(point);
	return true;
}


#endif
