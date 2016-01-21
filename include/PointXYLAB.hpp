#ifndef POINTXYLAB_HPP_
#define POINTXYLAB_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class PointXYLAB : public  PointXY
{
public:
  unsigned short values[3] = {0,0,0};
  
	PointXYLAB():PointXY(0,0)
	{
  
	}	

	PointXYLAB(unsigned short x_, unsigned short y_, unsigned short l_, unsigned short a_, unsigned short b_) : PointXY(x_,y_)
	{
  	values[0] = l_;
		values[1] = a_;
	  values[2] = b_;
	}

	std::string getSemanticColor()
	{
		return "none";
	}
};


#endif
