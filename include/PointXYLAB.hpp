#ifndef POINTXYLAB_HPP_
#define POINTXYLAB_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class PointXYLAB : public  PointXY
{
public:
  PointXYLAB(unsigned short, unsigned short,unsigned short,unsigned short,unsigned short);
  PointXYLAB();
  unsigned short values[3] = {0,0,0};
  
};

PointXYLAB::PointXYLAB():PointXY(0,0)
{
  
}

PointXYLAB::PointXYLAB(unsigned short x_, unsigned short y_, unsigned short l_, unsigned short a_, unsigned short b_) : PointXY(x_,y_)
{
  values[0] = l_;
  values[1] = a_;
  values[2] = b_;
}

#endif
