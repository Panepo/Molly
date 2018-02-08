#include "stdafx.h"
#include "realsense.h"


realsense::realsense()
{
	align = rs2::align(RS2_STREAM_DEPTH);
	pipeline = rs2::pipeline();
	config = rs2::config();
}

void realsense::setRes(int steam, int width, int height)
{
	switch (steam)
	{
	case RS2_STREAM_COLOR:
		colorWidth = width;
		colorHeight = height;
		break;
	case RS2_STREAM_INFRARED:
		infWidth = width;
		infHeight = height;
		break;
	case RS2_STREAM_DEPTH:
		depthWidth = width;
		depthHeight = height;
		break;
	}
}

void realsense::startStream()
{
	/*rs2::decimation_filter dec;
	dec.set_option(RS2_OPTION_FILTER_MAGNITUDE, 2);

	rs2::disparity_transform depth2disparity;
	rs2::disparity_transform disparity2depth(false);

	rs2::spatial_filter spat;
	spat.set_option(RS2_OPTION_HOLES_FILL, 5);*/

	config.enable_stream(RS2_STREAM_DEPTH, depthWidth, depthHeight, RS2_FORMAT_Z16, 30);
	config.enable_stream(RS2_STREAM_COLOR, colorWidth, colorHeight, RS2_FORMAT_RGBA8, 30);

	rs2::pipeline_profile profile = pipeline.start();
	auto sensor = profile.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == "High Density")
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

}

void realsense::getFrame()
{
	rs2::frameset frameset = pipeline.wait_for_frames();
	auto processed = align.process(frameset);

	rs2::video_frame colorFrame = processed.get_color_frame();
	rs2::depth_frame depthFrame = processed.get_depth_frame();
	depthColorMap = colorMap(depthFrame);

}

