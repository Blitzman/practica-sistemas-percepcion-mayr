#ifndef IMAGECOLORSEGMENTATION_HPP_
#define IMAGECOLORSEGMENTATION_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "PointXYHSL.hpp"
#include "Shape.hpp"



class ImageColorSegmentation
{

private:
    int ics_format;            // Whether IMAGE or VIDEO input
    std::string path;          // Path to resource
    cv::Mat image_;            // Current image or frame
    cv::Mat image_color_;      // Converted to HSL
    cv::Mat sure_bg_;          // Sure Background area
    cv::Mat sure_fg_;          // Sure Foreground area
    cv::Mat unknown_;          // Unknown area between surefg and surebg

    cv::VideoCapture vcap;      // Video stream

    cv::Mat processFrame();     // Process the current frame


public:

    static const int ICS_IMAGE = 0;        // Image source
    static const int ICS_VIDEO = 1;        // Video source

    ImageColorSegmentation(int ics_format, std::string path);

    bool process(cv::Mat &mat);            // Process the current frame and returns the color mask
    

};

ImageColorSegmentation::ImageColorSegmentation(int ics_format, std::string path)
{
    this->ics_format = ics_format;
    this->path = path;

    if(ics_format == 0) // FRAME OR SINGLE IMAGE
    {
        image_ = cv::imread(path);

    } else {    // VIDEO OR IMAGESET

        vcap = cv::VideoCapture(path);
    }
}

cv::Mat ImageColorSegmentation::processFrame()
{

    if (!image_.data)
    {
        std::cout << "No image data \n";
        exit(0);
    }


    //cv::Size size(2048,1536);
        //resize(image_, image_, size);


    image_.copyTo(image_color_);

    // Convert color image to HSV Scheme for easier color classfication
    cv::Mat color_HSV;
    cvtColor(image_color_, color_HSV, cv::COLOR_BGR2HLS);

    // Convert to grayscale
    cvtColor(image_, image_, cv::COLOR_BGR2GRAY );

    // Tresholding image binary inverted and otsu's method
    cv::threshold(image_, image_, 0, 255, cv::THRESH_BINARY_INV+cv::THRESH_OTSU);


    // Removing noises made by shadows/lights by opening
    cv::Mat kernel = cv::Mat::ones(cv::Size(3,3), CV_8U);
    cv::Mat opening;
    morphologyEx(image_, opening, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2);

    // Sure BG area, by applying mat morph (dilate) you assure that BG is in fact BG
    cv::dilate(opening, sure_bg_, kernel, cv::Point(1,1), 3);

    // Sure foreground area by calculing the distance to the closest zero pixel for each pixel and thresholding by the max distance
    cv::Mat dist_transform;
    distanceTransform(opening, dist_transform, CV_DIST_L2, 5);
    double min, max;
    cv::minMaxLoc(dist_transform, &min, &max);
    cv::threshold(dist_transform, sure_fg_, 0.5*max, 255, 0);

    // Geeting unknown region bu subtracting the sureFG from sureBG
    sure_fg_.convertTo(sure_fg_, CV_8U);
    subtract(sure_bg_, sure_fg_, unknown_);

    // Labelling the blobs
    cv::Mat markers;
    connectedComponents (sure_fg_, markers); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
    cv::minMaxLoc(markers, &min, &max);
    // std::cout << "There are " << max << " different shapes" << std::endl;

    // Adding 1 to every label to make the background 1 instead of 0
    cv::Size tam = markers.size();
    for(int i = 0; i < tam.height; i++)
    {
        for(int j = 0; j < tam.width; j++)
        {
            markers.at<int>(i,j) = markers.at<int>(i,j) + 1;
        }
    }

    // making unknown zones to be labelled as 0
    for(int i = 0; i < tam.height; i++)
    {
        for(int j = 0; j < tam.width; j++)
        {
            if (unknown_.at<uchar>(i,j) == 255)
                markers.at<int>(i,j) = 0;
        }
    }

    // Watershed algorithm
    cv::watershed(image_color_, markers);



















    std::vector<cv::Mat> hslChan;
    cv::split(color_HSV, hslChan);

    std::vector<cv::Mat> outputs(3);
    outputs[0] = cv::Mat(hslChan[0].size().height, hslChan[0].size().width, hslChan[0].type()); // R
    outputs[1] = cv::Mat(hslChan[0].size().height, hslChan[0].size().width, hslChan[0].type()); // G
    outputs[2] = cv::Mat(hslChan[0].size().height, hslChan[0].size().width, hslChan[0].type()); // B

    for(int i = 0; i < hslChan[0].size().height; i++)
    {
        for(int j = 0; j < hslChan[0].size().width; j++)
        {
            if(markers.at<int>(i,j) > 1) // HLS
            {
                if (hslChan[1].at<uchar>(i,j) < 20) {outputs[0].at<uchar>(i,j) = 0;}
                if (hslChan[1].at<uchar>(i,j) > 80) {outputs[0].at<uchar>(i,j) = 0;}
                if (hslChan[2].at<uchar>(i,j) < 63) {outputs[0].at<uchar>(i,j) = 0;}

                if(hslChan[0].at<uchar>(i,j) < 40 )
                {
                    outputs[0].at<uchar>(i,j) = 255;
                }
                else if(hslChan[0].at<uchar>(i,j) < 80 )
                {
                    outputs[1].at<uchar>(i,j) = 255;
                }
                else if(hslChan[0].at<uchar>(i,j) < 150 )
                {
                    outputs[2].at<uchar>(i,j) = 255;
                }
                else
                    outputs[0].at<uchar>(i,j) = 255;

            }
        }
    }




    cv::dilate(outputs[0], outputs[0], kernel, cv::Point(1,1), 3);
    cv::dilate(outputs[1], outputs[1], kernel, cv::Point(1,1), 3);
    cv::dilate(outputs[2], outputs[2], kernel, cv::Point(1,1), 3);

    // Tresholding image binary inverted and otsu's method
    cv::threshold(outputs[0], outputs[0], 0, 255, cv::THRESH_OTSU);
    cv::threshold(outputs[1], outputs[1], 0, 255, cv::THRESH_OTSU);
    cv::threshold(outputs[2], outputs[2], 0, 255, cv::THRESH_OTSU);


    // Filtro gaussiana o mediana

    /*cv::erode(outputs[0], outputs[0], kernel, cv::Point(1,1), 3);
    cv::erode(outputs[1], outputs[1], kernel, cv::Point(1,1), 3);
    cv::erode(outputs[2], outputs[2], kernel, cv::Point(1,1), 3);*/







    int nPiezasRojo = 0;
    int nPiezasVerde = 0;
    int nPiezasAzul = 0;
    max = 0;


    // Hay que actualizar para que si el primero va del 1 al 2, el siguiente tiene que ir del 2 a l 3
    cv::Mat markersRed;
    connectedComponents (outputs[0], markersRed); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
    cv::minMaxLoc(markersRed, &min, &max);
    std::cout << max << std::endl;

    nPiezasRojo = max;

    cv::Mat markersGreen;
    connectedComponents (outputs[1], markersGreen); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
    for(int i = 0; i < markersGreen.size().height; i++)
    {
        for(int j = 0; j < markersGreen.size().width; j++)
        {
            if(markersGreen.at<int>(i,j) > 0)
                markersGreen.at<int>(i,j) = markersGreen.at<int>(i,j) + max;
        }
    }
    cv::minMaxLoc(markersGreen, &min, &max);
    std::cout << max << std::endl;
    if(max != 0)
        nPiezasVerde = max;
 
    cv::Mat markersBlue;
    connectedComponents (outputs[2], markersBlue); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
    for(int i = 0; i < markersBlue.size().height; i++)
    {
        for(int j = 0; j < markersBlue.size().width; j++)
        {
            if(markersBlue.at<int>(i,j) > 0)
                markersBlue.at<int>(i,j) = markersBlue.at<int>(i,j) + max;
        }
    }
    cv::minMaxLoc(markersBlue, &min, &max);
    std::cout << max << std::endl;
    if(max != 0)
        nPiezasAzul = max;

    
    cv::Mat markers2;// = markersRed + markersGreen + markersBlue; // Si hay zonas solapadas falla porque la zona 1 + la zona 2 da 3 y en realidad son otras zonas pero solapadas
    markersBlue.copyTo(markers2);
    for(int i = 0; i < tam.height; i++)
    {
        for(int j = 0; j < tam.width; j++)
        {
            if (markersRed.at<int>(i,j) > 0)
            {
                markers2.at<int>(i,j) = markersRed.at<int>(i,j);
            } else if (markersGreen.at<int>(i,j) > 0)
            {
                markers2.at<int>(i,j) = markersGreen.at<int>(i,j);
            } else if (markersBlue.at<int>(i,j) > 0)
            {
                markers2.at<int>(i,j) = markersBlue.at<int>(i,j);
            } else
                markers2.at<int>(i,j) = 0;
        }
    }
    cv::minMaxLoc(markers2, &min, &max);
    std::cout << max << std::endl;



    // List of shapes
    int nPiezas = max;//nPiezasRojo + nPiezasVerde + nPiezasAzul;
    std::vector<Shape> shapes(nPiezas);

    // Masking the image with color per BG and SHAPES
    cv::Mat color_mask;
    image_color_.copyTo(color_mask);

    // Creating the color mask by coloring shapes and BG
    for(int i = 0; i < tam.height; i++)
    {
        for(int j = 0; j < tam.width; j++)
        {
            for(int l = 1; l <= nPiezas; l++)
            {
                if(l == markers2.at<int>(i,j) ) // Esto es que hay pieza
                {
                    PointXYHSL p;
                    if( l > 0 and l <= nPiezasRojo )
                    {
                        p = PointXYHSL(i,j, 10,0,0);
                    } else if ( l > 0 and l <= nPiezasVerde )
                    {
                        p = PointXYHSL(i,j, 70,0,0);  
                    }
                    else if ( l > 0 and l <= nPiezasAzul )
                    {
                        p = PointXYHSL(i,j, 100,0,0);  
                    }
                    // PointXYHSL p(i,j, color_HSV.at<cv::Vec3b>(i,j)[0], color_HSV.at<cv::Vec3b>(i,j)[1], color_HSV.at<cv::Vec3b>(i,j)[2]);
                    shapes[l-1].push_back(p);
                }
            }

        }
    }


    //std::vector<Shape> shapesMod(max);
    for(int i = 0; i < shapes.size(); i++) // color classification, centroid calcs and outputs
    {
        if(shapes[i].pointList.size() < 500)
        {
            shapes.erase(shapes.begin() + i);
            i--;
        }
    } // */ 


    std::cout << "Numero de piezas " << shapes.size() << std::endl;

    

    for(int i = 0; i < shapes.size(); i++) // color classification, centroid calcs and outputs
    {
        cv::Point c = shapes[i].getCentroid();
        std::string semColor = shapes[i].getSemanticAverageColor();

        std::cout << i << " (" << c.x << ", " << c.y << ") " << semColor << " " << shapes[i].getAverageColor() << " " << shapes[i].pointList.size() << std::endl;

        circle(color_mask, c, 10, 0, -1);

        putText(color_mask, std::to_string(i) + semColor, c, cv::FONT_HERSHEY_SIMPLEX, 5, cvScalar(0,0,0), 5);

    }

    outputs[0] /= 5;
    outputs[1] /= 3;
    outputs[2] /= 1;

    cv::Mat output = outputs[0] + outputs[1] + outputs[2];
    return color_mask;

}

bool ImageColorSegmentation::process(cv::Mat &frame)
{

    if(ics_format == 1) // VIDEO stream
    {
        image_ = cv::Mat();
        if(vcap.read(image_))
        {
            if (!image_.data)
            {
                std::cout << "No image data \n";
                return false;
            }

        }
        else 
            return false;
    }

    frame = processFrame();
 
    return true;
}



#endif
