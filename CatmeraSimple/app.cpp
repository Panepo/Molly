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

	//pixelA[0] = (float)ColorWidth * 3 / 5;
	//pixelA[1] = (float)ColorHeight / 2;
	//pixelB[0] = (float)ColorWidth * 2 / 5;
	//pixelB[1] = (float)ColorHeight / 2;

	// Skips some frames to allow for auto-exposure stabilization
	for (int i = 0; i < 10; i++) pipeline.wait_for_frames();
}

void app::cameraProcess()
{
	//begin = clock();
	cv::setMouseCallback(windowTitle, eventMouseS, this);
	
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
	case APPSTATE_MEASURER:
		stateMeasurer();
		break;
	default:
		state = APPSTATE_EXIT;
		break;
	}

	//end = clock();
	//elapsed = double(end - begin) * 1000 / CLOCKS_PER_SEC;
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

void app::eventMouseS(int event, int x, int y, int flags, void* userdata)
{
	app* temp = reinterpret_cast<app*>(userdata);
	temp->eventMouse(event, x, y, flags);
}

void app::eventMouse(int event, int x, int y, int flags)
{
	int modx, mody;

	if (stream & EnableColor)
	{
		if (x >= 0 && x <= ColorWidth)
			modx = x;
		else
			modx = ColorWidth - 1;
		if (y >= 0 && y <= ColorHeight)
			mody = y;
		else
			mody = ColorHeight - 1;
	}
	else if (stream & EnableInfrared)
	{
		if (x >= 0 && x <= DepthWidth)
			modx = x;
		else
			modx = DepthWidth - 1;
		if (y >= 0 && y <= DepthHeight)
			mody = y;
		else
			mody = DepthHeight - 1;
	}

	switch (state)
	{
	case APPSTATE_COLOR:
	case APPSTATE_INFRARED:
	case APPSTATE_DEPTH:
		streamEventHandler(event, modx, mody, flags);
		break;
	case APPSTATE_RULER:
		rulerEventHandler(event, modx, mody, flags);
		break;
	case APPSTATE_SCANNER:
		scannerEventHandler(event, modx, mody, flags);
		break;
	case APPSTATE_MEASURER:
		measurerEventHandler(event, modx, mody, flags);
		break;
	default:
		break;
	}
}