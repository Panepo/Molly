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
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), contourSorter());
	
	//std::cout << "fq: " << cv::contourArea(contours[0]) << std::endl;
	if (cv::contourArea(contours[0]) > scanMinSize)
	{
		cv::RotatedRect boundingBox = cv::minAreaRect(contours[0]);
		cv::Point2f corners[4];
		boundingBox.points(corners);
		cv::line(*input, corners[0], corners[1], scanRectColor, scanRectSize);
		cv::line(*input, corners[1], corners[2], scanRectColor, scanRectSize);
		cv::line(*input, corners[2], corners[3], scanRectColor, scanRectSize);
		cv::line(*input, corners[3], corners[0], scanRectColor, scanRectSize);
	}
}