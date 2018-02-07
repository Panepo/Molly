// Molirisi.cpp : Defines the entry point for the console application.
//
#include <iostream>

#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <librealsense2\rs.hpp>
#include <librealsense2\rsutil.h>


int main(int argc, char * argv[]) try
{
	//Contruct a pipeline which abstracts the device
	rs2::pipeline pipe;

	//Create a configuration for configuring the pipeline with a non default profile
	rs2::config cfg;

	//Add desired streams to configuration
	cfg.enable_stream(RS2_STREAM_INFRARED, 1280, 720, RS2_FORMAT_Y8, 30);
	cfg.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);

	//Instruct pipeline to start streaming with the requested configuration
	pipe.start(cfg);

	const auto ir_window_name = "IR image";
	cv::namedWindow(ir_window_name, cv::WINDOW_AUTOSIZE);

	const auto depth_window_name = "Depth image";
	cv::namedWindow(depth_window_name, cv::WINDOW_AUTOSIZE);
	
	while (true)
	{
		rs2::frameset data = pipe.wait_for_frames();

		//Get each frame
		rs2::frame ir_frame = data.first(RS2_STREAM_INFRARED);
		rs2::frame depth_frame = data.get_depth_frame();

		// Creating OpenCV matrix from IR image
		cv::Mat ir(cv::Size(1280, 720), CV_8UC1, (void*)ir_frame.get_data(), cv::Mat::AUTO_STEP);

		// Apply Histogram Equalization
		//cv::equalizeHist(ir, ir);
		//cv::applyColorMap(ir, ir, cv::COLORMAP_JET);

		
		cv::Mat depth(cv::Size(1280, 720), CV_16UC1, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);

		cv::convertScaleAbs(depth, depth, 0.1, 0);
		//cv::equalizeHist(depth, depth);
		cv::applyColorMap(depth, depth, cv::COLORMAP_JET);

		// Display the image in GUI
		
		cv::imshow("IR image", ir);
		cv::imshow("Depth image", depth);

		if (cv::waitKey(1) == 'q') {
			break;
		}
	}
	
	return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}
