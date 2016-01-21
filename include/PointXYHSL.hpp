#ifndef POINTXYHSL_HPP_
#define POINTXYHSL_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class PointXYHSL : public PointXY
{
public:
  PointXYHSL(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short);
  PointXYHSL();
  unsigned short values[3] = {0,0,0};
  
};

PointXYHSL::PointXYHSL():PointXY(0,0)
{
  
}

PointXYHSL::PointXYHSL(unsigned short x_, unsigned short y_, unsigned short h_, unsigned short s_, unsigned short v_): PointXY(x_,y_)
{
  values[0] = h_;
  values[1] = s_;
  values[2] = v_;
}


#endif
