#include "stdafx.h"
#include "app.h"

app::app()
{
	state = APPSTATE_EXIT;
}

void app::stateColor(rs2::pipeline pipeline, rs2::colorizer colorize)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat colorMat = formatTrans::frame2Mat(alignedFrame.get_color_frame());

	cv::imshow(WindowTitle, colorMat);
}

void app::stateInfrared(rs2::pipeline pipeline, rs2::colorizer colorize)
{
	rs2::align alignTo(RS2_STREAM_DEPTH);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat infraredMat = formatTrans::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));

	cv::imshow(WindowTitle, infraredMat);
}

void app::stateDepth(rs2::pipeline pipeline, rs2::colorizer colorize, int stream)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	if (stream & EnableInfrared)
		rs2::align alignTo(RS2_STREAM_INFRARED);

	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::frame depth = alignedFrame.get_depth_frame();
	rs2::frame depthColor = colorize(depth);
	cv::Mat depthMat = formatTrans::frame2Mat(depthColor);

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
		{
			state = APPSTATE_DEPTH;
		}
		else if (state == APPSTATE_INFRARED)
		{
			state = APPSTATE_DEPTH;
		}
		else if (state == APPSTATE_DEPTH)
		{
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
		}
	}
}
