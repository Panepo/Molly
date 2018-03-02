#include "stdafx.h"
#include "app.h"

// =================================================================================
// Application minor private functions for measurer
// =================================================================================

void app::measurePointer(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin)
{
	float posA[2], posB[2];

	if (scaleZoom == 1)
	{
		posA[0] = pixelA[0];
		posA[1] = pixelA[1];
		posB[0] = pixelB[0];
		posB[1] = pixelB[1];
	}
	else
	{
		posA[0] = pixelA[0] * scaleZoom + roiZoom[0];
		posA[1] = pixelA[1] * scaleZoom + roiZoom[1];
		posB[0] = pixelB[0] * scaleZoom + roiZoom[0];
		posB[1] = pixelB[1] * scaleZoom + roiZoom[1];
	}

	cv::circle(*input, cv::Point((int)pixel[0], (int)pixel[1]), pointerSize, pointerColor, -1);
	cv::circle(*input, cv::Point((int)pixelA[0], (int)pixelA[1]), measureSize, measureColor, 2);
	cv::circle(*input, cv::Point((int)pixelB[0], (int)pixelB[1]), measureSize, measureColor, 2);
	cv::line(*input, cv::Point((int)pixelA[0], (int)pixelA[1]),
		cv::Point((int)pixelB[0], (int)pixelB[1]), measureColor, 2);

	float dist = measureDist(intrin, depth, posA, posB);

	std::ostringstream strs;
	strs << dist;
	distText = strs.str() + "cm";

	int textCoordX = (int)((pixelA[0] + pixelB[0]) / 2);
	int textCoordY = (int)((pixelA[1] + pixelB[1]) / 2);

	cv::putText(*input, distText, cv::Point(textCoordX, textCoordY),
		pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(*input, distText, cv::Point(textCoordX, textCoordY),
		pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
}

float app::measureDist(const rs2_intrinsics* intrin, const rs2::depth_frame* depth, float pixelA[3], float pixelB[3])
{
	float pointA[3] = { 0, 0, 0 };
	float pointB[3] = { 0, 0, 0 };

	float distA = depth->get_distance((int)pixelA[0], (int)pixelA[1]);
	float distB = depth->get_distance((int)pixelB[0], (int)pixelB[1]);

	rs2_deproject_pixel_to_point(pointA, intrin, pixelA, distA);
	rs2_deproject_pixel_to_point(pointB, intrin, pixelB, distB);

	return floor(sqrt(pow(pointA[0] - pointB[0], 2) +
		pow(pointA[1] - pointB[1], 2) +
		pow(pointA[2] - pointB[2], 2)) * 10000) / 100;
}

void app::measureDrawer(cv::Mat * input, const rs2::depth_frame * depth)
{
	float posA[2], posB[2];

	if (scaleZoom == 1)
	{
		posA[0] = pixelA[0];
		posA[1] = pixelA[1];
		posB[0] = pixelB[0];
		posB[1] = pixelB[1];
	}
	else
	{
		posA[0] = pixelA[0] * scaleZoom + roiZoom[0];
		posA[1] = pixelA[1] * scaleZoom + roiZoom[1];
		posB[0] = pixelB[0] * scaleZoom + roiZoom[0];
		posB[1] = pixelB[1] * scaleZoom + roiZoom[1];
	}

	float xdiff = abs(posA[0] - posB[0]);
	float ydiff = abs(posA[1] - posB[1]);
	int posX = 0, posY = 0;
	float dist = 0, parm = 1;
	float directX = 1, directY = 1;
	std::vector<float> output;

	// get depth data of the route
	if (posA[0] - posB[0] < 0)
		directX = -1;

	if (posA[1] - posB[1] < 0)
		directY = -1;

	if (xdiff < ydiff)
	{
		for (int i : boost::irange<int>(0, (int)ydiff))
		{
			posX = (int)floor(posB[0] + directX * (float)i * xdiff / ydiff);
			posY = (int)floor(posB[1] + directY * (float)i);
			dist = depth->get_distance(posX, posY);
			output.push_back(dist);
		}
	}
	else
	{
		for (int i : boost::irange<int>(0, (int)xdiff))
		{
			posX = (int)floor(posB[0] + directX * (float)i);
			posY = (int)floor(posB[1] + directY * (float)i * ydiff / xdiff);
			dist = depth->get_distance(posX, posY);
			output.push_back(dist);
		}
	}

	if (output.size() > 0)
	{
		// get min and max value to calculate drawing parameter
		auto outMinMax = std::minmax_element(output.begin(), output.end());

		if ((float)(*outMinMax.second - *outMinMax.first) < 0.2)
			parm = sectionHeight * 5;
		else
			parm = sectionHeight / (*outMinMax.second - *outMinMax.first);

		// draw sectional drawing
		cv::Mat minimap = cv::Mat(sectionHeight, (int)output.size(), CV_8UC3, sectionColor);
		for (int i : boost::irange<int>(0, (int)output.size()))
		{
			cv::line(minimap, cv::Point(i, 0),
				cv::Point(i, (int)((output[i] - *outMinMax.first) * parm)), sectionBGColor, 1);
		}

		// flip image if inverse
		if (posA[0] < posB[0])
		{
			cv::Mat minimapFlip;
			cv::flip(minimap, minimapFlip, 1);
			minimap = minimapFlip.clone();
		}

		// generate preserve border
		cv::copyMakeBorder(minimap, minimap, sectionPreSize, 0, 0, 0, cv::BORDER_CONSTANT, sectionBGColor);
		cv::copyMakeBorder(minimap, minimap, 0, sectionPreSize, 0, 0, cv::BORDER_CONSTANT, sectionColor);

		// generate minimap
		cv::Size size = input->size();
		cv::Size sizeMap = cv::Size((int)(size.width / 8), (int)(size.height / 8));
		cv::resize(minimap, minimap, sizeMap, 0, 0, CV_INTER_LINEAR);
		cv::copyMakeBorder(minimap, minimap, sectionMapSize, sectionMapSize, sectionMapSize,
			sectionMapSize, cv::BORDER_CONSTANT, sectionMapColor);

		cv::Mat outMat = input->clone();
		minimap.copyTo(outMat(cv::Rect(size.width - sizeMap.width - 10, size.height - sizeMap.height - 10,
			minimap.cols, minimap.rows)));
		*input = outMat.clone();
	}
}

