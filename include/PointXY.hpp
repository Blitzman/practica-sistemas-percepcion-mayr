#ifndef POINTXY_HPP_
#define POINTXY_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

class PointXY
{
public:
  unsigned short x;
  unsigned short y;
  unsigned short values[3] = {0,0,0};
  
  PointXY()
  {
    x = 0;
    y = 0;
  }
  
  PointXY(unsigned short x, unsigned short y, unsigned short value0, unsigned short value1, unsigned short value2)
  {
    this->x = x;
    this->y = y;
    values[0] = value0;
    values[1] = value1;
    values[2] = value2;
  }

  std::string getSemanticColorHSV()
  {
    float hue = values[0];
    float sat = values[2];
    float lgt = values[1];

    if (lgt < 0.2)  return "Black";
    //if (lgt > 0.8)  return "Whites";

    if (sat < 0.25) return "Gray";

    if (hue < 30)   return "Red";
    //if (hue < 90)   return "Yellows";
    if (hue < 150)  return "Green";
    if (hue < 210)  return "Cyan";
    if (hue < 270)  return "Blue";
    if (hue < 330)  return "Magenta";
  }

  std::string getSemantivColorLAB()
  {
    return "NONE";
  }
};


#endif
