#include "stdafx.h"
#include "app.h"

app::app(std::string title)
{
	windowTitle = title;
	
	stream = detectDevice();
	if (stream & EnableColor)
	{
		config.enable_stream(RS2_STREAM_COLOR, ColorWidth, ColorHeight, RS2_FORMAT_BGR8, ColorFPS);
		state = APPSTATE_COLOR;
	}

	if (stream & EnableInfrared)
	{
		config.enable_stream(RS2_STREAM_INFRARED, DepthWidth, DepthHeight, RS2_FORMAT_Y8, DepthFPS);
		state = APPSTATE_INFRARED;
	}

	if (stream & EnableDepth)
		config.enable_stream(RS2_STREAM_DEPTH, DepthWidth, DepthHeight, RS2_FORMAT_Z16, DepthFPS);

	rs2::pipeline_profile cfg = pipeline.start(config);

	auto sensor = cfg.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == visualPreset)
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

	auto profile = cfg.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
	intrinsics = profile.get_intrinsics();

	cv::namedWindow(windowTitle, cv::WINDOW_AUTOSIZE);
}

void app::process()
{
	std::clock_t begin = clock();
	
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
	}

	eventKeyboard();
	clock_t end = clock();
	elapsed = double(end - begin) * 1000 / CLOCKS_PER_SEC;
}

void app::stateColor()
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = spat_filter.process(depth);
	depth = temp_filter.process(depth);
	
	cv::setMouseCallback(windowTitle, eventMouseS, this);
	streamPointer(&colorMat, &depth, &intrinsics);
	
	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms (" + std::to_string((int)point[0]) + " " + std::to_string((int)point[1]) + " " + std::to_string((int)point[2]) + ")";
	streamInfoer(&colorMat, str);

	cv::imshow(windowTitle, colorMat);
}

void app::stateInfrared()
{
	rs2::align alignTo(RS2_STREAM_INFRARED);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = spat_filter.process(depth);
	depth = temp_filter.process(depth);
	
	cv::setMouseCallback(windowTitle, eventMouseS, this);
	streamPointer(&infraredMat, &depth, &intrinsics);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms (" + std::to_string((int)point[0]) + " " + std::to_string((int)point[1]) + " " + std::to_string((int)point[2]) + ")";
	streamInfoer(&infraredMat, str);

	cv::imshow(windowTitle, infraredMat);
}

void app::stateDepth()
{
	rs2::colorizer colorize;
	//colorize.set_option(RS2_OPTION_COLOR_SCHEME, 2);
	
	rs2::align alignTo(RS2_STREAM_COLOR);
	if (stream & EnableInfrared)
		rs2::align alignTo(RS2_STREAM_INFRARED);

	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = spat_filter.process(depth);
	depth = temp_filter.process(depth);
	rs2::frame depthColor = colorize(depth);
	cv::Mat depthMat = funcFormat::frame2Mat(depthColor);

	cv::setMouseCallback(windowTitle, eventMouseS, this);
	streamPointer(&depthMat, &depth, &intrinsics);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms (" + std::to_string((int)point[0]) + " " + std::to_string((int)point[1]) + " " + std::to_string((int)point[2]) + ")";
	streamInfoer(&depthMat, str);

	cv::imshow(windowTitle, depthMat);
}

void app::eventKeyboard()
{
	char key = cv::waitKey(10);

	if (key == 'q' || key == 'Q')
		state = APPSTATE_EXIT;
	else if (key == 's' || key == 'S')
	{
		if (state == APPSTATE_COLOR)
			state = APPSTATE_DEPTH;
		else if (state == APPSTATE_INFRARED)
			state = APPSTATE_DEPTH;
		else if (state == APPSTATE_DEPTH)
		{
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
		}
	}
}

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

void app::eventMouseS(int event, int x, int y, int flags, void* userdata)
{
	app* temp = reinterpret_cast<app*>(userdata);
	temp->eventMouse(event, x, y, flags);
}

void app::eventMouse(int event, int x, int y, int flags)
{
	if (event == CV_EVENT_MOUSEMOVE)
	{
		pixel[0] = (float)x;
		pixel[1] = (float)y;
	}
}

void app::streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin)
{
	auto depthPixel = depth->get_distance((int)pixel[0], (int)pixel[1]);
	rs2_deproject_pixel_to_point(point, intrin, pixel, depthPixel * 1000);

	cv::circle(*input, cv::Point((int)pixel[0], (int)pixel[1]), pointerSize, pointerColor, -1);
	std::string text = std::to_string((int)point[0]) + " " + std::to_string((int)point[1]) + " " + std::to_string((int)point[2]);

	cv::putText(*input, text, cv::Point((int)pixel[0] - 100, (int)pixel[1] + 40), pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(*input, text, cv::Point((int)pixel[0] - 100, (int)pixel[1] + 40), pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
}

void app::streamInfoer(cv::Mat* input, std::string text)
{
	cv::Size size = input->size();
	cv::copyMakeBorder(*input, *input, 0, 40, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	cv::putText(*input, text, cv::Point(10, size.height + 30), inforerFont, 1, inforerColor, 1, cv::LINE_AA);
}
