#include "stdafx.h"
#include "app.h"
#include "funcOpenCV.h"
#include "funcGeometry3D.h"

void app::measurerMain(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, measurerState& mstate)
{
	//std::cout << mstate << std::endl;
	switch (mstate)
	{
	case MEASURER_INIT:
		break;
	case MEASURER_PAINT:
		measurerPaint(input);
		break;
	case MEASURER_RECT:
		measurerRect(input, depth, intrin);
		break;
	case MEASURER_CALC:
		measurerCalc(input, depth, intrin);
		break;
	case MEASURER_RESET:
		break;
	default:
		break;
	}
}

void app::measurerInit()
{

	mstate = MEASURER_INIT;
}

void app::measurerReset()
{

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
	cv::Mat inputRoiGray;
	cv::cvtColor(inputRoi, inputRoiGray, CV_RGB2GRAY);
	cv::GaussianBlur(inputRoiGray, inputRoiGray, cv::Size(7, 7), 0, 0);
	cv::Mat inputRoiEdge;
	cv::Canny(inputRoiGray, inputRoiEdge, 50, 150, 3);

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

				cv::RotatedRect boundingBox = cv::minAreaRect(contours[i]);
				cv::Point2f corners[4];
				boundingBox.points(corners);

				float a1[2], a2[2], b1[2], b2[2];
				a1[0] = (corners[0].x + corners[1].x) / 2;
				a2[0] = (corners[2].x + corners[3].x) / 2;
				a1[1] = (corners[0].y + corners[1].y) / 2;
				a2[1] = (corners[2].y + corners[3].y) / 2;

				b1[0] = (corners[0].x + corners[3].x) / 2;
				b2[0] = (corners[1].x + corners[2].x) / 2;
				b1[1] = (corners[0].y + corners[3].y) / 2;
				b2[1] = (corners[1].y + corners[2].y) / 2;

				cv::line(inputRoi, cv::Point(a1[0], a1[1]), cv::Point(a2[0], a2[1]), measLineColor, measLineSize);
				cv::line(inputRoi, cv::Point(b1[0], b1[1]), cv::Point(b2[0], b2[1]), measLineColor, measLineSize);

				float distA = funcGeometry3D::calcDist3D(a1, a2, depth, intrin);
				float distB = funcGeometry3D::calcDist3D(b1, b2, depth, intrin);
				float area = floor(distA * distB * 100) / 100;

				std::ostringstream strs;
				strs << area;
				distText = strs.str() + "cm2";



				/*cv::line(inputRoi, corners[0], corners[1], measRectColor2, measLineSize);
				cv::line(inputRoi, corners[1], corners[2], measRectColor2, measLineSize);
				cv::line(inputRoi, corners[2], corners[3], measRectColor2, measLineSize);
				cv::line(inputRoi, corners[3], corners[0], measRectColor2, measLineSize);*/

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

void app::measurerCalc(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
}

