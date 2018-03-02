#include "stdafx.h"
#include "app.h"

// =================================================================================
// Application minor private functions for streamer
// =================================================================================

void app::streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin)
{
	float pos[2];

	if (scaleZoom == 1)
	{
		pos[0] = pixel[0];
		pos[1] = pixel[1];
	}
	else
	{
		pos[0] = pixel[0] * scaleZoom + roiZoom[0];
		pos[1] = pixel[1] * scaleZoom + roiZoom[1];
	}

	float depthPixel = depth->get_distance((int)pos[0], (int)pos[1]);
	rs2_deproject_pixel_to_point(point, intrin, pos, depthPixel * 1000);

	cv::circle(*input, cv::Point((int)pixel[0], (int)pixel[1]), pointerSize, pointerColor, -1);
	std::string text = std::to_string((int)point[0]) + " " + std::to_string((int)point[1])
		+ " " + std::to_string((int)point[2]);

	cv::putText(*input, text, cv::Point((int)pixel[0] - 100, (int)pixel[1] + 40), pointerFontA,
		1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(*input, text, cv::Point((int)pixel[0] - 100, (int)pixel[1] + 40), pointerFontB,
		1, pointerColorFB, 1, cv::LINE_AA);
}

void app::streamInfoer(cv::Mat* input, std::string text)
{
	cv::Size size = input->size();
	cv::copyMakeBorder(*input, *input, 0, 40, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	cv::putText(*input, text, cv::Point(10, size.height + 30), inforerFont, 1, inforerColor, 1, cv::LINE_AA);
}

cv::Mat app::streamZoomer(cv::Mat* input)
{
	if (scaleZoom == 1)
	{
		cv::Mat output = input->clone();
		return output;
	}
	else
	{
		// generate zoom-in image
		pixelZoom[0] = (int)(pixelZoom[0] * scaleZoom) + roiZoom[0];
		pixelZoom[1] = (int)(pixelZoom[1] * scaleZoom) + roiZoom[1];

		cv::Size size = input->size();
		int scaledWidth = (int)(size.width * scaleZoom);
		int scaledHeight = (int)(size.height * scaleZoom);
		roiZoom[0] = pixelZoom[0] - (scaledWidth / 2);
		roiZoom[1] = pixelZoom[1] - (scaledHeight / 2);

		if ((pixelZoom[0] + (scaledWidth / 2)) > size.width)
			roiZoom[0] = size.width - scaledWidth;

		if ((pixelZoom[1] + (scaledHeight / 2)) > size.height)
			roiZoom[1] = size.height - scaledHeight;

		if ((pixelZoom[0] - (scaledWidth / 2)) < 0)
			roiZoom[0] = 0;

		if ((pixelZoom[1] - (scaledHeight / 2)) < 0)
			roiZoom[1] = 0;

		cv::Rect roi = cv::Rect(roiZoom[0], roiZoom[1], scaledWidth, scaledHeight);
		cv::Mat output = (*input)(roi);
		cv::resize(output, output, size, 0, 0, CV_INTER_AREA);

		// generate mini-map
		cv::Mat outMap = input->clone();
		cv::rectangle(outMap, roi, zoomerLineColor, zoomerLineSize);
		cv::Size sizeMap = cv::Size((int)(size.width / 8), (int)(size.height / 8));
		cv::resize(outMap, outMap, sizeMap, 0, 0, CV_INTER_AREA);
		cv::copyMakeBorder(outMap, outMap, zoomerMapSize, zoomerMapSize, zoomerMapSize,
			zoomerMapSize, cv::BORDER_CONSTANT, zoomerMapColor);
		outMap.copyTo(output(cv::Rect(10, size.height - sizeMap.height - 10,
			outMap.cols, outMap.rows)));

		return output;
	}
}

