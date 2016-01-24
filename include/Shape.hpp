#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class Shape
{
	private:
		// std::vector<PointXYHSL> pointList;

	public:
		//Shape(int t);
		Shape();
		Shape(std::vector<PointXY>);
		cv::Point getCentroid();
		cv::Vec3i getAverageColor();
		bool push_back(PointXY);
        std::string getSemanticAverageColorHLS();
        std::string getSemanticAverageColorLAB();

        std::vector<PointXY> pointList;


};

Shape::Shape()
{

}

Shape::Shape(std::vector<PointXY> v)
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

std::string Shape::getSemanticAverageColorHLS()
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

std::string Shape::getSemanticAverageColorLAB()
{
    cv::Vec3i values = getAverageColor();
    float lig = values[0];
    float channelA = values[1];
    float channelB = values[2];

    float limitsRed[6] = 
      {
        85.0,160.0,160.0,100.0,171.0,171.0
      };
    
    float limitsGreen[6] =
      {
        48,-34,38,58,-43,48
      };

    float limitsBlue[6] =
      {
        20,-2,-24,30,8,-34
      };
    
    // std::cout<<"1: " <<lig <<"\t 2: " <<channelA <<"\t 3: " <<channelB <<std::endl;
    if(lig > limitsRed[0] && lig < limitsRed[3] &&  channelA > limitsRed[1] && channelA < limitsRed[4] && channelB > limitsRed[2] && channelB < limitsRed[5]) return "Red";
    if(lig > limitsBlue[0] && lig < limitsBlue[3] &&  channelA > limitsBlue[1] && channelA < limitsBlue[4] && channelB > limitsBlue[2] && channelB < limitsBlue[5]) return "Blue";
    if(lig > limitsGreen[0] && lig < limitsGreen[3] &&  channelA > limitsGreen[1] && channelA < limitsGreen[4] && channelB > limitsGreen[2] && channelB < limitsGreen[5]) return "Green";
    
    return "NONE";
  
}

cv::Vec3i Shape::getAverageColor()
{
	cv::Vec3i colorAvg;
	for(int i = 0; i < pointList.size(); i++)
	{
		colorAvg[0] += pointList[i].values[0];
        colorAvg[1] += pointList[i].values[1];
        colorAvg[2] += pointList[i].values[2];
	}
	colorAvg[0] /= pointList.size();
	colorAvg[1] /= pointList.size();
	colorAvg[2] /= pointList.size();

	return colorAvg;
}

bool Shape::push_back(PointXY point)
{
	pointList.push_back(point);
	return true;
}


#endif
