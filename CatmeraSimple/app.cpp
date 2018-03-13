#include "stdafx.h"
#include "app.h"

app::app(std::string title)
{
	windowTitle = title;
}

void app::cameraInitial()
{
	stream = detectDevice();
	if (stream & EnableColor)
	{
		config.enable_stream(RS2_STREAM_COLOR, ColorWidth, ColorHeight, RS2_FORMAT_BGR8, ColorFPS);
		state = APPSTATE_COLOR;
		//state = APPSTATE_RULER;
	}
	else if (stream & EnableInfrared)
	{
		config.enable_stream(RS2_STREAM_INFRARED, DepthWidth, DepthHeight, RS2_FORMAT_Y8, DepthFPS);
		state = APPSTATE_INFRARED;
	}

	config.enable_stream(RS2_STREAM_DEPTH, DepthWidth, DepthHeight, RS2_FORMAT_Z16, DepthFPS);

	rs2::pipeline_profile cfg = pipeline.start(config);

	auto sensor = cfg.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == visualPreset)
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

	auto profile = cfg.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();
	
	if (stream & EnableInfrared)
		profile = cfg.get_stream(RS2_STREAM_INFRARED).as<rs2::video_stream_profile>();
	
	intrinsics = profile.get_intrinsics();

	filterSpat.set_option(RS2_OPTION_HOLES_FILL, 5);

	pixelA[0] = (float)ColorWidth * 3 / 5;
	pixelA[1] = (float)ColorHeight / 2;
	pixelB[0] = (float)ColorWidth * 2 / 5;
	pixelB[1] = (float)ColorHeight / 2;

	// Skips some frames to allow for auto-exposure stabilization
	for (int i = 0; i < 10; i++) pipeline.wait_for_frames();
}

void app::cameraProcess()
{
	std::clock_t begin = clock();
	//cv::setMouseCallback(windowTitle, eventMouseS, this);
	
	switch (state)
	{
	case APPSTATE_COLOR:
		stateColor();
		break;
	case APPSTATE_INFRARED:
		stateInfrared();
		break;
	case APPSTATE_DEPTH:
		stateDepth();
		break;
	case APPSTATE_RULER:
		stateRuler();
		break;
	case APPSTATE_PHOTOGRAPHER:
		statePhotographer();
		break;
	case APPSTATE_SCANNER:
		stateScanner();
		break;
	default:
		state = APPSTATE_EXIT;
		break;
	}

	//eventKeyboard();
	clock_t end = clock();
	elapsed = double(end - begin) * 1000 / CLOCKS_PER_SEC;
}

// =================================================================================
// Application settings
// =================================================================================

void app::setResolution(int stream, int width, int height, int fps)
{
	switch (stream)
	{
	case RS2_STREAM_COLOR:
		ColorWidth = width;
		ColorHeight = height;
		ColorFPS = fps;
		break;
	case RS2_STREAM_INFRARED:
	case RS2_STREAM_DEPTH:
		DepthWidth = width;
		DepthHeight = height;
		DepthFPS = fps;
		break;
	default:
		break;
	}
}


void app::setVisualPreset(std::string preset)
{
	visualPreset = preset;
}

// =================================================================================
// Application events
// =================================================================================

void app::eventMouseCV(int event, int x, int y, int flags, int wheel)
{
	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		switch (state)
		{
		case APPSTATE_COLOR:
			if (x >= 0 && x <= ColorWidth)
				pixel[0] = (float)x;
			if (y >= 0 && y <= ColorHeight)
				pixel[1] = (float)y;
			break;
		case APPSTATE_INFRARED:
			if (x >= 0 && x <= DepthWidth)
				pixel[0] = (float)x;
			if (y >= 0 && y <= DepthHeight)
				pixel[1] = (float)y;
			break;
		case APPSTATE_DEPTH:
		case APPSTATE_RULER:
			if (stream & EnableColor)
			{
				if (x >= 0 && x <= ColorWidth)
					pixel[0] = (float)x;
				if (y >= 0 && y <= ColorHeight)
					pixel[1] = (float)y;
			}
			else if (stream & EnableInfrared)
			{
				if (x >= 0 && x <= DepthWidth)
					pixel[0] = (float)x;
				if (y >= 0 && y <= DepthHeight)
					pixel[1] = (float)y;
			}
			break;
		default:
			break;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		switch (state)
		{
		case APPSTATE_RULER:
			if (stream & EnableColor)
			{
				if (x >= 0 && x <= ColorWidth)
					pixelA[0] = (float)x;
				else
					pixelA[0] = (float)ColorWidth - 1;
				if (y >= 0 && y <= ColorHeight)
					pixelA[1] = (float)y;
				else
					pixelA[1] = (float)ColorHeight - 1;
			}
			else if (stream & EnableInfrared)
			{
				if (x >= 0 && x <= DepthWidth)
					pixelA[0] = (float)x;
				else
					pixelA[0] = (float)DepthWidth - 1;
				if (y >= 0 && y <= DepthHeight)
					pixelA[1] = (float)y;
				else
					pixelA[1] = (float)DepthHeight - 1;
			}
			break;
		}
		break;
	case CV_EVENT_RBUTTONDOWN:
		switch (state)
		{
		case APPSTATE_RULER:
			if (stream & EnableColor)
			{
				if (x >= 0 && x <= ColorWidth)
					pixelB[0] = (float)x;
				else
					pixelB[0] = (float)ColorWidth - 1;
				if (y >= 0 && y <= ColorHeight)
					pixelB[1] = (float)y;
				else
					pixelB[1] = (float)ColorHeight - 1;
			}
			else if (stream & EnableInfrared)
			{
				if (x >= 0 && x <= DepthWidth)
					pixelB[0] = (float)x;
				else
					pixelB[0] = (float)DepthWidth - 1;
				if (y >= 0 && y <= DepthHeight)
					pixelB[1] = (float)y;
				else
					pixelB[1] = (float)DepthHeight - 1;
			}
			break;
		}
		break;
	case CV_EVENT_MOUSEWHEEL:
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
		case APPSTATE_DEPTH:
		case APPSTATE_RULER:
		case APPSTATE_SCANNER:
			float pos[2];
			
			if (scaleZoom == 1)
			{
				pos[0] = x;
				pos[1] = y;
			}
			else
			{
				pos[0] = x * scaleZoom + roiZoom[0];
				pos[1] = y * scaleZoom + roiZoom[1];
			}
			
			pixelZoom[0] = pos[0];
			pixelZoom[1] = pos[1];

			if (wheel > 0 && scaleZoom < zoomerScaleMax)
				scaleZoom += (float) 0.1;
			else if (wheel < 0 && scaleZoom > zoomerScaleMin)
				scaleZoom -= (float) 0.1;
			break;
		default:
			break;
		}

	default:
		break;
	}
}

