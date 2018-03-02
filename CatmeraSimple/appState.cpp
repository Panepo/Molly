#include "stdafx.h"
#include "app.h"

// =================================================================================
// Application states
// =================================================================================

void app::stateColor()
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);

	streamPostProcess(&colorMat, &depth);
}

void app::stateInfrared()
{
	rs2::align alignTo(RS2_STREAM_INFRARED);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);

	streamPostProcess(&infraredMat, &depth);
}

void app::stateDepth()
{
	rs2::colorizer colorize;
	colorize.set_option(RS2_OPTION_COLOR_SCHEME, 0);

	rs2::align alignTo(RS2_STREAM_COLOR);
	if (stream & EnableInfrared)
		rs2::align alignTo(RS2_STREAM_INFRARED);

	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);
	rs2::frame depthColor = colorize(depth);
	cv::Mat depthMat = funcFormat::frame2Mat(depthColor);

	streamPostProcess(&depthMat, &depth);
}

void app::stateMeasure()
{
	rs2::frameset data = pipeline.wait_for_frames();

	if (stream & EnableColor)
	{
		rs2::align alignTo(RS2_STREAM_COLOR);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);
		measurePostProcess(&colorMat, &depth);
	}
	else
	{
		rs2::align alignTo(RS2_STREAM_INFRARED);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);
		measurePostProcess(&infraredMat, &depth);
	}
}

void app::stateAlign()
{
	rs2::colorizer colorize;
	colorize.set_option(RS2_OPTION_COLOR_SCHEME, 2);
	
	rs2::frameset data = pipeline.wait_for_frames();

	if (stream & EnableColor)
	{
		rs2::align alignTo(RS2_STREAM_COLOR);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);
		rs2::frame depthColor = colorize(depth);
		cv::Mat depthMat = funcFormat::frame2Mat(depthColor);
		alignPostProcess(&colorMat, &depthMat);
	}
}

// =================================================================================
// Application major private functions
// =================================================================================

void app::streamPostProcess(cv::Mat* input, rs2::depth_frame* depth)
{
	outputMat = streamZoomer(input);

	streamPointer(&outputMat, depth, &intrinsics);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms (" + std::to_string((int)point[0]) + " "
		+ std::to_string((int)point[1]) + " " + std::to_string((int)point[2]) + ")";
	streamInfoer(&outputMat, str);
}

void app::measurePostProcess(cv::Mat * input, rs2::depth_frame * depth)
{
	outputMat = streamZoomer(input);

	measurePointer(&outputMat, depth, &intrinsics);
	measureDrawer(&outputMat, depth);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms " + distText;
	streamInfoer(&outputMat, str);
}

void app::alignPostProcess(cv::Mat * input, cv::Mat * depth)
{
	alignRenderer(input, depth);
	outputMat = streamZoomer(input);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms " + distText;
	streamInfoer(&outputMat, str);
}
