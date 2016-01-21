#ifndef POINTXYHSL_HPP_
#define POINTXYHSL_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "PointXY.hpp"

class PointXYHSL : public PointXY
{
public:
  unsigned short values[3] = {0,0,0};
  
	PointXYHSL():PointXY(0,0)
	{
  
	}

	PointXYHSL(unsigned short x_, unsigned short y_, unsigned short h_, unsigned short s_, unsigned short v_): PointXY(x_,y_)
	{
		values[0] = h_;
		values[1] = s_;
	  values[2] = v_;
	}

	std::string getSemanticColor()
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
};

#endif
