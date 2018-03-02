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

void app::scanRenderer(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	cv::Mat inputGray;
	cv::cvtColor(*input, inputGray, CV_RGB2GRAY);
	cv::GaussianBlur(inputGray, inputGray, cv::Size(3, 3), 0, 0);

	cv::Mat inputEdge;
	cv::Canny(inputGray, inputEdge, 75, 200, 3);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

	std::sort(contours.begin(), contours.end(), contourSorter());
	std::reverse(contours.begin(), contours.end());
	
	std::vector<std::vector<cv::Point>> approx;
	double peri = 0;
	for (int i : boost::irange<int>(0, (int)contours.size()))
	{
		peri = cv::arcLength(contours[i], true);
		cv::approxPolyDP(contours[i], approx[0], 0.02 * peri, true);

		if (approx[0].size() == 4)
		{
			cv::drawContours(*input, approx, -1, scanRectColor, scanRectSize);
			break;
		}	
	}
}