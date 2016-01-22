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
  float values[3] = {0,0,0};
  
  PointXY()
  {
    x = 0;
    y = 0;
  }
  
  PointXY(unsigned short x, unsigned short y, float value0, float value1, float value2)
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

  std::string getSemanticColorLAB()
  {
    float lig = values[0];
    float channelA = values[1];
    float channelB = values[2];

    float limitsRed[6] = 
      {
        85.0,160.0,160.0,100.0,171.0,171.0
      };
    
    
    std::cout<<"1: " <<lig <<"\t 2: " <<channelA <<"\t 3: " <<channelB <<std::endl;
    if(lig > limitsRed[0] && lig < limitsRed[3] &&  channelA > limitsRed[1] && channelA < limitsRed[4] && channelB > limitsRed[2] && channelB < limitsRed[5]) return "Red";
    
    return "NONE";
  }
};


#endif
