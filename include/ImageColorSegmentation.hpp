#ifndef IMAGECOLORSEGMENTATION_HPP_
#define IMAGECOLORSEGMENTATION_HPP_

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "PointXY.hpp"
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

    int areaFilter;

    bool showHistogram;

    cv::Mat processFrameHLV(cv::Mat &color_HSV, cv::Mat &markers, std::vector<Shape> &shapes);     // Process the current frame
    cv::Mat processFrameLAB(cv::Mat &markers, double max);     // Process the current frame
    cv::Mat preProcessFrame(int hls_lab, std::vector<Shape> &shapes);


public:
    ImageColorSegmentation(std::string path);
    bool process(int hls_lab, cv::Mat &mat, std::vector<Shape> &shapes);            // Process the current frame and returns the color mask
    void setHistogramOutput(bool);                      // Wether show histogram or not
    void setAreaFilter(int areaFilter);

    static int HLS;
    static int LAB;

};

int ImageColorSegmentation::HLS = 0;
int ImageColorSegmentation::LAB = 1;

ImageColorSegmentation::ImageColorSegmentation(std::string path)
{
    this->path = path;
    showHistogram = false;
    areaFilter = 1000;
    image_ = cv::imread(path);

    if (!image_.data)
    {
        std::cout << "No image data \n";
        return;
    }
}

void ImageColorSegmentation::setHistogramOutput(bool v)
{
    showHistogram = v;
}

void ImageColorSegmentation::setAreaFilter(int v)
{
    areaFilter = v;
}

cv::Mat ImageColorSegmentation::preProcessFrame(int hls_lab, std::vector<Shape> &shapes)
{
    // Reading the frame
    if (!image_.data)
    {
        std::cout << "No image data \n";
        exit(0);
    }

    image_.copyTo(image_color_);

    // Convert color image to HSV Scheme for easier color classfication
    cv::Mat color_HSV;
    cvtColor(image_color_, color_HSV, cv::COLOR_BGR2HLS);

    // Working on HLS image
    std::vector<cv::Mat> hslChan2;
    cv::split(color_HSV, hslChan2); // Splitting channels


    // Calculate histogram, Avg and deviation for automatic BG subtraction
    int histSize = 256;    // bin size
    float range[] = { 0, 255 };
    const float *ranges[] = { range };
 
    cv::MatND hist;
    cv::calcHist( &hslChan2[1], 1, 0, cv::Mat(), hist, 1, &histSize, ranges, true, false );

    double total;
    int globalMax = 0;
    int globalMaxIdx = 0;
    total = image_.rows * image_.cols;
    double avg;
    double xi2Fi;
    double deviation;
    for( int h = 0; h < histSize; h++ )
     {
        float binVal = hist.at<float>(h);

        avg += h*binVal;
        xi2Fi +=h*h*binVal;

        if (binVal > globalMax)
        {
            globalMax = binVal;
            globalMaxIdx = h;
        }
     }
     avg/=total;
     deviation = sqrt( xi2Fi/total - avg*avg );
     std::cout<<globalMaxIdx<<" " <<avg<< " " << deviation <<std::endl;;
 
    // Plot the histogram
    int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );
 
    cv::Mat histImage( hist_h, hist_w, CV_8UC1, cv::Scalar( 0,0,0) );
    cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );

    if(showHistogram)
    {
        for( int i = 1; i < histSize; i++ )
        {
          cv::line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
                           cv::Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
                           cv::Scalar( 255, 0, 0), 2, 8, 0  );
        }

        cv::line( histImage, cv::Point( bin_w*(avg), hist_h ) ,
                           cv::Point( bin_w*(avg), hist_h - 1000 ),
                           cv::Scalar( 255, 0, 0), 2, 8, 0  );
        cv::line( histImage, cv::Point( bin_w*(avg+deviation), hist_h ) ,
                           cv::Point( bin_w*(avg+deviation), hist_h - 1000 ),
                           cv::Scalar( 255, 0, 0), 2, 8, 0  );
        cv::line( histImage, cv::Point( bin_w*(avg-deviation), hist_h ) ,
                           cv::Point( bin_w*(avg-deviation), hist_h - 1000 ),
                           cv::Scalar( 255, 0, 0), 2, 8, 0  );
 
        cv::namedWindow( "Result", 1 );    cv::imshow( "Result", histImage );
        cv::waitKey(25);
    }


     // Correcting the BG by whitening the values between [avg+deviation, avg-deviation]
    for(int i = 0; i < image_.size().height; i++)
    {
        for(int j = 0; j < image_.size().width; j++)
        {
            if(color_HSV.at<cv::Vec3b>(i,j)[1] < avg+deviation && color_HSV.at<cv::Vec3b>(i,j)[1] > avg-deviation)
            {
                color_HSV.at<cv::Vec3b>(i,j)[1] = 200;
            }
        }
    } 

    // Back to RGB
    cvtColor(color_HSV, image_, cv::COLOR_HLS2BGR);

    // And greyscale 
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
    cv::threshold(dist_transform, sure_fg_, 0.3*max, 255, 0); // Esto se cepilal el triangulillo

    // Geeting unknown region bu subtracting the sureFG from sureBG
    sure_fg_.convertTo(sure_fg_, CV_8U);
    subtract(sure_bg_, sure_fg_, unknown_);

    // Labelling the blobs
    cv::Mat markers;
    connectedComponents (sure_fg_, markers); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
    cv::minMaxLoc(markers, &min, &max);

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

    cv::Mat ret;
        if(hls_lab == 0)
            ret = processFrameHLV(color_HSV, markers, shapes);
        else if (hls_lab == 1)
            ret = processFrameLAB(markers, max);


    return ret;
}


// Working hard on the image for color segmentation
cv::Mat ImageColorSegmentation::processFrameHLV(cv::Mat &color_HSV, cv::Mat &markers, std::vector<Shape> &shapes)
{

    // Working on HLS image
    std::vector<cv::Mat> hslChan;
    cv::split(color_HSV, hslChan); // Splitting channels

    std::vector<cv::Mat> outputs(3);
    outputs[0] = cv::Mat(hslChan[0].size().height, hslChan[0].size().width, hslChan[0].type()); // R
    outputs[1] = cv::Mat(hslChan[0].size().height, hslChan[0].size().width, hslChan[0].type()); // G
    outputs[2] = cv::Mat(hslChan[0].size().height, hslChan[0].size().width, hslChan[0].type()); // B

    // Filtering the image to extract red, green or blue data

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

    cv::Mat kernel = cv::Mat::ones(cv::Size(3,3), CV_8U);

    // Dilate to delete rebel pixels in the borders of the shapes
    cv::dilate(outputs[0], outputs[0], kernel, cv::Point(1,1), 3);
    cv::dilate(outputs[1], outputs[1], kernel, cv::Point(1,1), 3);
    cv::dilate(outputs[2], outputs[2], kernel, cv::Point(1,1), 3);

    // Tresholding image otsu's method
    cv::threshold(outputs[0], outputs[0], 0, 255, cv::THRESH_OTSU);
    cv::threshold(outputs[1], outputs[1], 0, 255, cv::THRESH_OTSU);
    cv::threshold(outputs[2], outputs[2], 0, 255, cv::THRESH_OTSU);



    int nPiezasRojo = 0;
    int nPiezasVerde = 0;
    int nPiezasAzul = 0;
    double min, max;


    // Tagging the blobs for each color data and updating such tags values
    cv::Mat markersRed;
    connectedComponents (outputs[0], markersRed); // Labels the BG as 0 and every blob by numbers 1, 2, 3 ...
    cv::minMaxLoc(markersRed, &min, &max);

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
    if(max != 0)
        nPiezasAzul = max;

    // This is a simple sum of matrices but if there are overlayed zones caused by the dilating process it puts the correct tag
    // The common sum will create new/fake tags
    cv::Mat markers2;
    markersBlue.copyTo(markers2);
    cv::Size tam = markers2.size();
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


    // List of shapes
    int nPiezas = max; 
    //std::vector<Shape> shapes(nPiezas);
    shapes = std::vector<Shape> (nPiezas);

    // Masking the image with color per BG and SHAPES
    cv::Mat color_mask;
    image_color_.copyTo(color_mask);

    // Extracting the shapes by checking only the positions where there are blobs (in grayscale mode)
    for(int i = 0; i < tam.height; i++)
    {
        for(int j = 0; j < tam.width; j++)
        {
            for(int l = 1; l <= nPiezas; l++)
            {
                if(l == markers2.at<int>(i,j) ) // This means this pixel is part of a blob / shape
                {                               // This filter boosts the processing time
                    PointXY p;
                    if( l > 0 and l <= nPiezasRojo )
                    {
                        p = PointXY(i,j, 10,0,0);
                    } else if ( l > 0 and l <= nPiezasVerde )
                    {
                        p = PointXY(i,j, 70,0,0);  
                    }
                    else if ( l > 0 and l <= nPiezasAzul )
                    {
                        p = PointXY(i,j, 100,0,0);  
                    }
                    shapes[l-1].add_point(p);
                }
            }

        }
    }

    // Filtering those shapes which area does not meet a minimun size
    for(int i = 0; i < shapes.size(); i++) 
    {
        if(shapes[i].m_point_list.size() < areaFilter)
        {
            shapes.erase(shapes.begin() + i);
            i--;
        }
    } // */ 


    std::cout << "Numero de piezas " << shapes.size() << std::endl;

    // color classification, centroid calcs and outputs
    for(int i = 0; i < shapes.size(); i++) 
    {
			std::cout << "iteration " << i << "\n";
        cv::Point c = shapes[i].getCentroid();
				std::cout << "centroided\n";
        std::string semColor = shapes[i].getSemanticAverageColorHLS();

			std::cout << "semcolored\n";
        std::cout << i;
				std::cout	<< " (" << c.x;
				std::cout << ", " << c.y;
				std::cout << ") ";
				std::cout	<< semColor;
				std::cout	<< " ";
				std::cout	<< shapes[i].m_point_list.size();
				std::cout	<< "\n";

        circle(color_mask, c, 10, 0, -1);

        putText(color_mask, std::to_string(i) + semColor, c, cv::FONT_HERSHEY_SIMPLEX, 5, cvScalar(0,0,0), 5);

    }

    return color_mask;

}

cv::Mat ImageColorSegmentation::processFrameLAB(cv::Mat &markers, double max)
{
    cv::Size tam = markers.size();
    cv::Mat color_LAB;
    cv::Mat color_resized;
    /*resize(image_color_, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorHSVPrueba", cv::WINDOW_NORMAL);
    cv::imshow("ColorHSVPrueba", color_resized);*/
    // Convert color image to HSV Scheme for easier color classfication
   
    cvtColor(image_color_, color_LAB, cv::COLOR_BGR2Lab);
    //cv::Mat color_resized;
    resize(color_LAB, color_resized, cv::Size(640,480));
    cv::namedWindow("ColorLAB", cv::WINDOW_NORMAL);
    cv::imshow("ColorLAB", color_resized);
    
    // Masking the image with color per BG and SHAPES
    cv::Mat color_mask;
    image_color_.copyTo(color_mask);

    /*PROBEEE! for(int row = 0; row < color_LAB.rows;row++)
      {
        const uchar * ptr = (uchar *)color_LAB.data ;//+ row*color_LAB.)
        for(int col = 0; col < color_LAB.cols;col++)
          {
            std::cout<<"int = " <<std::endl <<" " <<(int)* ptr <<std::endl<<std::endl;
            
          }

      }
    */


      std::vector<Shape> shapes(max);


    // Creating the color mask by coloring shapes and BG
    for(int i = 0; i < tam.height; i++)
      {
        for(int j = 0; j < tam.width; j++)
          {
            if (markers.at<int>(i,j) == 1) // background
              {
                // color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(0,255,0);
              }

            for(int l = 2; l <= max+1; l++)
              {
                if (markers.at<int>(i,j) == l) // shape
                  {
                    PointXY p(i,j, color_LAB.at<cv::Vec3b>(i,j)[0], color_LAB.at<cv::Vec3b>(i,j)[1], color_LAB.at<cv::Vec3b>(i,j)[2]);

                    shapes[l-2].add_point(p);

                    // color_mask.at<cv::Vec3b>(i,j) = cv::Vec3b(127,0,0);
                  }
              }

          }
      }
    resize(color_mask, color_resized, cv::Size(640,480));
    //cv::namedWindow("ColorMask", cv::WINDOW_NORMAL);
    //cv::imshow("ColorMask", color_resized);
    for(int i = 0; i < max; i++) // color classification, centroid calcs and outputs
      {
        cv::Point c = shapes[i].getCentroid();
        std::string semColor = shapes[i].getSemanticAverageColorLAB();

        std::cout << "(" << c.x << ", " << c.y << ") " << semColor << std::endl;

        circle(color_mask, c, 10, 0, -1);
        putText(color_mask, semColor, c, cv::FONT_HERSHEY_SIMPLEX, 5, cvScalar(0,0,0), 5);
      }

    return color_mask;
}

// Calls the color segmentation process. Indirection level added for some preprocessing to be made here.
bool ImageColorSegmentation::process(int hls_lab, cv::Mat &frame, std::vector<Shape> &shapes)
{

    frame = preProcessFrame(hls_lab, shapes);
 
    return true;
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







#endif
