#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXYHSL.hpp"
#include "PointXYLAB.hpp"

template <class T> class Shape
{
private:
  unsigned short shape_format;
  std::vector<T> pointList;
  
public:
  //Shape(int t);
  Shape();
  // Shape(unsigned short shape_format);
  cv::Point getCentroid();
  cv::Vec3i getAverageColor();
  bool push_back(T);
  std::string getSemanticAverageColor();

};


template <class T>
Shape<T>::Shape()
{
}

/*template <class T>
Shape<T>::Shape(unsigned short shape_format)
{
}*/

template <class T>
cv::Point Shape<T>::getCentroid()
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

template <class T>
std::string Shape<T>::getSemanticAverageColor()
{
  cv::Vec3i c = getAverageColor();
  if(shape_format == 0)
    {
      
  float hue = c[0];
  float sat = c[2];
  float lgt = c[1];

  if (lgt < 0.2)  return "Black";
  //if (lgt > 0.8)  return "Whites";

  if (sat < 0.25) return "Gray";

  if (hue < 30)   return "Red";
  //if (hue < 90)   return "Yellows";
  if (hue < 150)  return "Green";
  if (hue < 210)  return "Cyan";
  if (hue < 270)  return "Blue";
  if (hue < 330)  return "Magent";
    }
  else
    {          
      float l = c[0];
      float a = c[2];
      float b = c[1];

      /*if (lgt < 0.2)  return "Black";
      //if (lgt > 0.8)  return "Whites";

      if (sat < 0.25) return "Gray";

      if (hue < 30)   return "Red";
      //if (hue < 90)   return "Yellows";
      if (hue < 150)  return "Green";
      if (hue < 210)  return "Cyan";
      if (hue < 270)  return "Blue";
      if (hue < 330)  return "Magent";*/
    }
  return "Red";
}

template <class T>
cv::Vec3i Shape<T>::getAverageColor()
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

template <class T>
bool Shape<T>::push_back(T point)
{
    pointList.push_back(point);
}

#endif
