// CatmeraSimple.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <iostream>
#include <opencv2/opencv.hpp>

#include "appState.h"
#include "formatTrans.h"
#include "cameraConfig.h"

int main(int argc, char * argv[]) try
{
	rs2::pipeline pipeline;
	rs2::config config;
	rs2::colorizer colorize;
	//colorize.set_option(RS2_OPTION_COLOR_SCHEME, 2);

	int stream = detectDevice();

	if (stream & EnableColor)
		config.enable_stream(RS2_STREAM_COLOR, ColorWidth, ColorHeight, RS2_FORMAT_BGR8, ColorFPS);
	
	if (stream & EnableInfrared)
		config.enable_stream(RS2_STREAM_INFRARED, DepthWidth, DepthHeight, RS2_FORMAT_Y8, DepthFPS);
	
	if (stream & EnableDepth)
		config.enable_stream(RS2_STREAM_DEPTH, DepthWidth, DepthHeight, RS2_FORMAT_Z16, DepthFPS);
		
	rs2::align alignTo(RS2_STREAM_DEPTH);
	rs2::pipeline_profile profile = pipeline.start(config);

	auto sensor = profile.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == VisualPreset)
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

	int appState = 1;
	
	while (appState)
	{
		rs2::frameset data = pipeline.wait_for_frames();
		rs2::frameset alignedFrame = alignTo.process(data);
		
		if (appState == 1)
		{
			//auto colorMat = frame_to_mat(alignedFrame.get_color_frame());

			rs2::frame depth = alignedFrame.get_depth_frame();

			rs2::frame depthColor = colorize(depth);

			auto depthMat = frame_to_mat(depthColor);

			//cv::imshow("Molly Catmera Color", colorMat);
			cv::imshow("Molly Catmera Depth", depthMat);
		}
		
		char key = cv::waitKey(10);
		if (key == 'q' || key == 'Q')
		{
			appState = 0;
		}
	}


	return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	system("pause");
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	system("pause");
	return EXIT_FAILURE;
}

