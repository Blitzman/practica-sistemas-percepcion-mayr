#include "ImageColorSegmentation.hpp"

// #include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <glob.h>
#include <unistd.h>

using namespace std;

std::vector<std::string> globVector(const string& pattern){
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
    vector<string> files;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        files.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return files;
}


int main(int argc, char** argv )
{

    /*
        Example of color segmentation for a video source (frames extracted to a folder)
    */

    std::vector<std::string> files = globVector("../resources/*");

    cv::Size size(640, 480);
    for(int i = 0; i < files.size(); i++)
    {
        cv::Mat frame;
        std::cout << files[i] << std::endl;
        ImageColorSegmentation ics(files[i]);
        ics.setHistogramOutput(false);
        ics.process(frame);
        cv::Mat color_resized;
        resize(frame, color_resized, size);
        cv::namedWindow( "DisplayWindow" ); // Create a window for display.
        cv::imshow("DisplayWindow", color_resized ); 
        cv::waitKey(25);
    }
    cv::waitKey(0); // */



    /*
    	Example of color segmentation for a still frame or image input
    */

    /*	ImageColorSegmentation ics("../resources/Picture_17.jpg");
    	cv::Mat frame;
    	ics.process(frame);
        cv::Mat color_resized;
        cv::Size size(640, 480);
        resize(frame, color_resized, size);
    	cv::namedWindow( "DisplayWindow" ); 
        cv::imshow("DisplayWindow", color_resized ); 
        cv::waitKey(0);

    // */


	return 0;
}
