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

	postStreamer(&colorMat, &depth);
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

	postStreamer(&infraredMat, &depth);
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

	postStreamer(&depthMat, &depth);
}

void app::stateRuler()
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

		postRuler(&colorMat, &depth);
	}
	else
	{
		rs2::align alignTo(RS2_STREAM_INFRARED);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);

		postRuler(&infraredMat, &depth);
	}
}

void app::statePhotographer()
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
		postPhotographer(&colorMat, &depthMat);
	}
	else
	{
		rs2::align alignTo(RS2_STREAM_INFRARED);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);
		rs2::frame depthColor = colorize(depth);
		cv::Mat depthMat = funcFormat::frame2Mat(depthColor);
		postPhotographer(&infraredMat, &depthMat);
	}
}

void app::stateScanner()
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

		postScanner(&colorMat, &depth);
	}
	else
	{
		rs2::align alignTo(RS2_STREAM_INFRARED);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);

		postScanner(&infraredMat, &depth);
	}
}

void app::stateMeasurer()
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

		postMeasurer(&colorMat, &depth);
	}
	else
	{
		rs2::align alignTo(RS2_STREAM_INFRARED);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);

		postMeasurer(&infraredMat, &depth);
	}
}

// =================================================================================
// Application major private functions
// =================================================================================

void app::postStreamer(cv::Mat* input, rs2::depth_frame* depth)
{
	begin = clock();
	outputMat = streamZoomer(input);

	float point[3] = { 0, 0, 0 };
	streamPointer(&outputMat, depth, &intrinsics, point);

	end = clock();
	elapsed = double(end - begin) * 1000000 / CLOCKS_PER_SEC;
	elapsedAvg = floor((elapsedAvg * 9 + elapsed) / 10);

	std::ostringstream strs;
	strs << elapsedAvg;
	std::string str = strs.str() + " us (" + std::to_string((int)point[0]) + " "
		+ std::to_string((int)point[1]) + " " + std::to_string((int)point[2]) + ")";
	streamInfoer(&outputMat, str);
}

void app::postRuler(cv::Mat * input, rs2::depth_frame * depth)
{
	begin = clock();
	outputMat = streamZoomer(input);

	rulerMain(&outputMat, depth, &intrinsics, rstate);

	end = clock();
	elapsed = double(end - begin) * 1000000 / CLOCKS_PER_SEC;
	elapsedAvg = floor((elapsedAvg * 9 + elapsed) / 10);

	std::ostringstream strs;
	strs << elapsedAvg;
	std::string str = strs.str() + " us " + distText;
	streamInfoer(&outputMat, str);
}

void app::postPhotographer(cv::Mat * input, cv::Mat * depth)
{
	begin = clock();
	photographerRenderer(input, depth);
	outputMat = streamZoomer(input);

	end = clock();
	elapsed = double(end - begin) * 1000000 / CLOCKS_PER_SEC;
	elapsedAvg = floor((elapsedAvg * 9 + elapsed) / 10);

	std::ostringstream strs;
	strs << elapsedAvg;
	std::string str = strs.str() + " us " + distText;
	streamInfoer(&outputMat, str);
}

void app::postScanner(cv::Mat * input, rs2::depth_frame * depth)
{
	begin = clock();
	outputMat = streamZoomer(input);
	scannerDrawer(&outputMat, depth, &intrinsics);

	end = clock();
	elapsed = double(end - begin) * 1000000 / CLOCKS_PER_SEC;
	elapsedAvg = floor((elapsedAvg * 9 + elapsed) / 10);

	std::ostringstream strs;
	strs << elapsedAvg;
	std::string str = strs.str() + " us " + distText;
	streamInfoer(&outputMat, str);
}

void app::postMeasurer(cv::Mat * input, rs2::depth_frame * depth)
{
	begin = clock();
	outputMat = streamZoomer(input);

	measurerMain(&outputMat, depth, &intrinsics, mstate);

	end = clock();
	elapsed = double(end - begin) * 1000000 / CLOCKS_PER_SEC;
	elapsedAvg = floor((elapsedAvg * 9 + elapsed) / 10);

	std::ostringstream strs;
	strs << elapsedAvg;
	std::string str = strs.str() + " us " + distText;
	streamInfoer(&outputMat, str);
}
