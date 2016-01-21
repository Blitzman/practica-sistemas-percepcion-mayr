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
  PointXY(unsigned short, unsigned short);
};

PointXY::PointXY(unsigned short x, unsigned short y)
{
  this->x = x;
  this->y = y;
}

#endif
