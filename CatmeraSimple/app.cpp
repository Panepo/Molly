#include "stdafx.h"
#include "app.h"

app::app()
{
	state = APPSTATE_EXIT;
	std::fill_n(pixel, 2, 0);
	std::fill_n(point, 3, 0);
}

void app::stateColor(rs2::pipeline* pipeline, rs2_intrinsics* intrin)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline->wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	
	cv::setMouseCallback(WindowTitle, getCoordPixelS, this);
	streamPointer(&colorMat, &depth, intrin);

	cv::imshow(WindowTitle, colorMat);
}

void app::stateInfrared(rs2::pipeline* pipeline, rs2_intrinsics* intrin)
{
	rs2::align alignTo(RS2_STREAM_DEPTH);
	rs2::frameset data = pipeline->wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	
	cv::setMouseCallback(WindowTitle, getCoordPixelS, this);
	streamPointer(&infraredMat, &depth, intrin);

	cv::imshow(WindowTitle, infraredMat);
}

void app::stateDepth(rs2::pipeline* pipeline, int stream, rs2_intrinsics* intrin)
{
	rs2::colorizer colorize;
	//colorize.set_option(RS2_OPTION_COLOR_SCHEME, 2);
	
	rs2::align alignTo(RS2_STREAM_COLOR);
	if (stream & EnableInfrared)
		rs2::align alignTo(RS2_STREAM_INFRARED);

	rs2::frameset data = pipeline->wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	rs2::frame depthColor = colorize(depth);
	cv::Mat depthMat = funcFormat::frame2Mat(depthColor);

	cv::setMouseCallback(WindowTitle, getCoordPixelS, this);
	streamPointer(&depthMat, &depth, intrin);

	cv::imshow(WindowTitle, depthMat);
}

void app::stateSwitch(int stream)
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

void app::getCoordPixelS(int event, int x, int y, int flags, void* userdata)
{
	app* temp = reinterpret_cast<app*>(userdata);
	temp->getCoordPixel(event, x, y, flags);
}

void app::getCoordPixel(int event, int x, int y, int flags)
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
