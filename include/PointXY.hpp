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
    float lgt = values[1];
    float sat = values[2];

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

  std::string getSemanticColorLAB()
  {
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
    
    std::cout<<"1: " <<lig <<"\t 2: " <<channelA <<"\t 3: " <<channelB <<std::endl;
    if(lig > limitsRed[0] && lig < limitsRed[3] &&  channelA > limitsRed[1] && channelA < limitsRed[4] && channelB > limitsRed[2] && channelB < limitsRed[5]) return "Red";
    if(lig > limitsBlue[0] && lig < limitsBlue[3] &&  channelA > limitsBlue[1] && channelA < limitsBlue[4] && channelB > limitsBlue[2] && channelB < limitsBlue[5]) return "Blue";
    if(lig > limitsGreen[0] && lig < limitsGreen[3] &&  channelA > limitsGreen[1] && channelA < limitsGreen[4] && channelB > limitsGreen[2] && channelB < limitsGreen[5]) return "Green";
    
    return "NONE";
  }
};


#endif
