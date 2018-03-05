#include "stdafx.h"
#include "app.h"

struct contourSorter
{
	bool operator ()(const std::vector<cv::Point>& a, const std::vector<cv::Point> & b)
	{
		double areaA = cv::contourArea(a);
		double areaB = cv::contourArea(b);
		return areaA > areaB;
	}
};

// =================================================================================
// Application minor private functions for scanner
// =================================================================================

void app::scannerDrawer(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	cv::Mat inputGray;
	cv::cvtColor(*input, inputGray, CV_RGB2GRAY);
	cv::threshold(inputGray, inputGray, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);

	cv::Mat inputEdge;
	cv::Canny(inputGray, inputEdge, 75, 200, 3);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), contourSorter());

	std::vector<cv::Point> approx;
	for (int i = 0; i < (int)contours.size(); i += 1)
	{
		cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);

		if ((int)approx.size() == 4)
		{
			cv::drawContours(*input, contours, i, scanRectColor, scanRectSize);
			break;
		}
		else if ((int)approx.size() == 5 || (int)approx.size() == 6)
		{
			cv::drawContours(*input, contours, i, scanRectColor, scanRectSize);
			cv::RotatedRect boundingBox = cv::minAreaRect(contours[0]);
			cv::Point2f corners[4];
			boundingBox.points(corners);
			cv::line(*input, corners[0], corners[1], sectionColor, scanRectSize);
			cv::line(*input, corners[1], corners[2], sectionColor, scanRectSize);
			cv::line(*input, corners[2], corners[3], sectionColor, scanRectSize);
			cv::line(*input, corners[3], corners[0], sectionColor, scanRectSize);
			break;
		}
	}
}