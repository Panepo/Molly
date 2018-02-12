// CatmeraSimple.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <iostream>
#include <opencv2/opencv.hpp>

#include "app.h"
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
		
	rs2::pipeline_profile profile = pipeline.start(config);

	auto sensor = profile.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == VisualPreset)
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);
	
	app catmera;
	if (stream & EnableColor)
		catmera.state = APPSTATE_COLOR;
	else
		catmera.state = APPSTATE_INFRARED;
	
	while (catmera.state)
	{
		if (catmera.state == APPSTATE_COLOR)
			catmera.stateColor(pipeline, colorize);
		else if (catmera.state == APPSTATE_INFRARED)
			catmera.stateInfrared(pipeline, colorize);
		else if (catmera.state == APPSTATE_DEPTH)
			catmera.stateDepth(pipeline, colorize, stream);
		
		catmera.stateSwitch(stream);
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

