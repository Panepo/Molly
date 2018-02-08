// CatmeraSimple.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <iostream>
#include <opencv2/opencv.hpp>

#include "opencvHelper.h"

int main(int argc, char * argv[]) try
{
	rs2::pipeline pipeline;
	rs2::config config;
	rs2::colorizer colorize;

	config.enable_stream(RS2_STREAM_DEPTH, 1280, 720, RS2_FORMAT_Z16, 30);
	config.enable_stream(RS2_STREAM_COLOR, 1280, 720, RS2_FORMAT_RGBA8, 30);

	rs2::pipeline_profile profile = pipeline.start();

	rs2::align alignTo(RS2_STREAM_COLOR);

	auto sensor = profile.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == "High Density")
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

	while (true)
	{
		rs2::frameset data = pipeline.wait_for_frames();
		rs2::frameset alignedFrame = alignTo.process(data);
		
		auto colorMat = frame_to_mat(alignedFrame.get_color_frame());
		
		rs2::frame depth = alignedFrame.get_depth_frame();
		//colorize.set_option(RS2_OPTION_COLOR_SCHEME, 2);
		rs2::frame depthColor = colorize(depth);

		auto depthMat = frame_to_mat(depthColor);

		cv::imshow("Molly Catmera Color", colorMat);
		cv::imshow("Molly Catmera Depth", depthMat);

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

