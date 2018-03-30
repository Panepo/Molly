#include "stdafx.h"
#include "app.h"
#include "funcOpenCV.h"
#include "funcGeometry3D.h"

void app::measurerMain(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, measurerState& mstate)
{
	//std::cout << mstate << std::endl;
	switch (mstate)
	{
	case MEASURER_WAIT:
		break;
	case MEASURER_PAINT:
		measurerPaint(input);
		break;
	case MEASURER_RECT:
		measurerRect(input, depth, intrin);
		break;
	default:
		break;
	}
}

void app::measurerPaint(cv::Mat * input)
{
	cv::Mat overlay = input->clone();
	cv::rectangle(overlay, pixelMeasureA, pixelMeasureB, measRectColor1, measRectSize1);
	cv::addWeighted(overlay, transparentP, *input, transparentO, 0, *input);
}

void app::measurerRect(cv::Mat * input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin)
{
	cv::Mat overlay = input->clone();

	cv::Rect roi = cv::Rect(pixelMeasureA, pixelMeasureB);
	cv::Mat inputRoi = overlay(roi);
	cv::Mat inputRoiEdge;
	funcOpenCV::cannyBlur(inputRoi, inputRoiEdge, 50, 150);
	//funcOpenCV::cannySharp(inputRoi, inputRoiEdge, 50, 150);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputRoiEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), funcOpenCV::contourSorter);

	if ((int)contours.size() > 0)
	{
		std::vector<cv::Point> approx;
		for (int i = 0; i < (int)contours.size(); i += 1)
		{
			cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);

			if ((int)approx.size() >= measApproxMin && (int)approx.size() <= measApproxMax)
			{
				cv::drawContours(inputRoi, contours, i, measContColor, measContSize);

				double area = funcGeometry3D::calcArea3D(contours[i], &inputRoi, depth, intrin, 
					pixelMeasureA, measLineColor, measLineSize);

				std::ostringstream strs;
				strs << area;
				distText = strs.str() + "cm2";

				break;
			}
		}

		inputRoi.copyTo(overlay(roi));

		cv::Point textCoord;
		textCoord.x = pixelMeasureB.x + 10;
		textCoord.y = pixelMeasureB.y + 10;

		cv::putText(*input, distText, textCoord, pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
		cv::putText(*input, distText, textCoord, pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
	}
	
	cv::rectangle(overlay, pixelMeasureA, pixelMeasureB, measRectColor2, measRectSize2);
	cv::addWeighted(overlay, transparentP, *input, transparentO, 0, *input);
}

void app::measurerEventHandler(int event, int x, int y, int flags)
{
	float value;

	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		if (mstate == MEASURER_PAINT)
		{
			pixelMeasureB.x = x;
			pixelMeasureB.y = y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		mstate = MEASURER_PAINT;
		pixelMeasureA.x = x;
		pixelMeasureA.y = y;
		pixelMeasureB.x = x;
		pixelMeasureB.y = y;
		break;
	case CV_EVENT_LBUTTONUP:
		mstate = MEASURER_RECT;
		break;
	case CV_EVENT_MOUSEWHEEL:
		pixelZoom[0] = x;
		pixelZoom[1] = y;

		value = (float)cv::getMouseWheelDelta(flags);
		//std::cout << value << std::endl;
		if (value > 0 && scaleZoom < zoomerScaleMax)
			scaleZoom += (float) 0.1;
		else if (value < 0 && scaleZoom > zoomerScaleMin)
			scaleZoom -= (float) 0.1;
		break;
	default:
		break;
	}
}

