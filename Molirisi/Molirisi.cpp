// Molirisi.cpp : Defines the entry point for the console application.
//
#include <iostream>

#include "stdafx.h"

#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include <librealsense2\rs.hpp>


int main()
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

	// Camera warmup - dropping several first frames to let auto-exposure stabilize
	rs2::frameset frames;
	for (int i = 0; i < 30; i++)
	{
		//Wait for all configured streams to produce a frame
		//frames = pipe.wait_for_frames();
	}

	//cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE);
	
	while (true)
	{
		frames = pipe.wait_for_frames();

		//Get each frame
		rs2::frame ir_frame = frames.first(RS2_STREAM_INFRARED);
		rs2::frame depth_frame = frames.get_depth_frame();

		// Creating OpenCV matrix from IR image
		cv::Mat ir(cv::Size(1280, 720), CV_8UC1, (void*)ir_frame.get_data(), cv::Mat::AUTO_STEP);

		// Apply Histogram Equalization
		cv::equalizeHist(ir, ir);
		cv::applyColorMap(ir, ir, cv::COLORMAP_JET);

		
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
	
	return 0;
}

