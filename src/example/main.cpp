#include "ImageShapeSegmentation.hpp"
#include "ImageColorSegmentation.hpp"
#include "Shape.hpp"

// #include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <glob.h>
#include <unistd.h>
#include <limits.h>

std::vector<std::string> globVector(const std::string& pattern){
    glob_t glob_result;
    glob(pattern.c_str(),GLOB_TILDE,NULL,&glob_result);
		std::vector<std::string> files;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        files.push_back(std::string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return files;
}

void merge_shapes(std::vector<Shape> & rShapesColor, std::vector<Shape> & rShapesShape)
{
	for (unsigned int i = 0; i < rShapesColor.size(); ++i)
	{
		unsigned int min_index_ = 0;
		double min_distance_ = std::numeric_limits<double>::max();

		cv::Point c_c_ = rShapesColor[i].getCentroid();

		for (unsigned int j = 0; j < rShapesShape.size(); ++j)
		{
			cv::Point c_s_ = rShapesShape[j].get_vertex_centroid();

			double distance_ = cv::norm(c_c_ - c_s_);

			if (distance_ < min_distance_)
			{
				min_distance_ = distance_;
				min_index_ = j;
			}
		}

		rShapesShape[min_index_].m_point_list = rShapesColor[i].m_point_list;
		std::cout << "Color[" << i << "] corresponds to Shape[" << min_index_ << "]...\n";
	}
}

int main(int argc, char** argv )
{
  std::vector<std::string> files = globVector("../resources/*");

  cv::Size size(640, 480);
  for(int i = 0; i < files.size(); i++)
  {
    cv::Mat frame;
    std::cout << files[i] << std::endl;
		std::vector<Shape> shapes_color_;
    ImageColorSegmentation ics(files[i]);
    ics.setHistogramOutput(false);
    ics.process(ImageColorSegmentation::HLS, frame, shapes_color_);
    cv::Mat color_resized;
    resize(frame, color_resized, size);
    cv::namedWindow( "DisplayWindow" ); // Create a window for display.
    cv::imshow("DisplayWindow", color_resized );

		cv::Mat frame_shape_;
		ImageShapeSegmentation iss_(files[i]);
		iss_.process(frame_shape_);
		cv::Mat shape_resized_;
		resize(frame_shape_, shape_resized_, size);
		cv::namedWindow("DisplayWindow2");
		cv::imshow("DisplayWindow2", shape_resized_);

		cv::Mat final_image_;
		final_image_ = cv::imread(files[i]);
		std::vector<Shape> shapes_shape_ = iss_.get_shapes();

		merge_shapes(shapes_color_, shapes_shape_);

		std::cout << "Drawing shapes...\n";

		for (Shape s_ : shapes_shape_)
		{
			s_.draw_name(final_image_, cv::Scalar(0, 0, 0));
			s_.draw_box(final_image_, cv::Scalar(0, 0, 0));
			s_.draw_contour(final_image_, cv::Scalar(0, 0, 0));
		}

		std::cout << "Showing image...\n";

		cv::Mat final_image_resized_;
		resize(final_image_, final_image_resized_, size);
		cv::namedWindow("Semantic Image");
		cv::imshow("Semantic Image", final_image_resized_);

		cv::waitKey(0);
  }
  cv::waitKey(0); // */

	return 0;
}
