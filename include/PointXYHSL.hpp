#ifndef POINTXYHSL_HPP_
#define POINTXYHSL_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

class PointXYHSL
{
	public:
		int x;
		int y;
		int h;
		int s;
		int v;
		PointXYHSL(int, int, int, int, int);

};

PointXYHSL::PointXYHSL(int x_, int y_, int h_, int s_, int v_)
{
	x = x_;
	y = y_;
	h = h_;
	s = s_;
	v = v_;
}




#endif