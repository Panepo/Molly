#include "stdafx.h"
#include "app.h"
#include "funcGeometry2D.h"
#include "funcOpenCV.h"

bool contourSorter(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2)
{
	double i = fabs(cv::contourArea(cv::Mat(contour1)));
	double j = fabs(cv::contourArea(cv::Mat(contour2)));
	return (i > j);
}

// =================================================================================
// Application minor private functions for scanner
// =================================================================================

void app::scannerDrawer(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	cv::Mat inputGray;
	cv::cvtColor(*input, inputGray, CV_RGB2GRAY);
	//cv::threshold(inputGray, inputGray, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);

	cv::Mat structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4));
	morphologyEx(inputGray, inputGray, cv::MORPH_OPEN, structuringElmt);
	morphologyEx(inputGray, inputGray, cv::MORPH_CLOSE, structuringElmt);
	cv::GaussianBlur(inputGray, inputGray, cv::Size(7, 7), 0, 0);

	cv::Mat inputEdge;
	cv::Canny(inputGray, inputEdge, 75, 200, 3);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), contourSorter);

	std::vector<cv::Point> approx;
	for (int i = 0; i < (int)contours.size(); i += 1)
	{
		cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);

		if ((int)approx.size() == 4)
		{
			double sideA = funcGeometry2D::calcDist2D(approx[1], approx[0]);
			double sideB = funcGeometry2D::calcDist2D(approx[1], approx[2]);
			double sideX = sideA / sideB;
			
			if (sideX >= 0.25 && sideX <= 4)
			{
				cv::Mat warped;
				funcOpenCV::fourPointTransform(*input, warped, approx);

				cv::Mat output;
				cv::Size size = input->size();
				cv::Size sizeW = warped.size();
				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);
				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);

				funcOpenCV::addMinimapRD(*input, warped, output, sizeMap, scanMapSize, scanMapColor);		
				*input = output.clone();
				cv::drawContours(*input, contours, i, scanRectColor, scanRectSize);
			}
			break;
		}
		/*else if ((int)approx.size() == 5)
		{
			cv::drawContours(*input, contours, i, scanRectColor, scanRectSize);

			std::vector<cv::Point> poly5;
			
			if (funcGeometry::convertPoly5to4(approx, poly5))
			{
				for (int j = 0; j < (int)poly5.size()-1; j += 1)
					cv::line(*input, poly5[i], poly5[i+1], sectionColor, scanRectSize);
			}
			break;
		}*/
		else if ((int)approx.size() >= 5 || (int)approx.size() <= 6)
		{
			cv::Mat warped;
			funcOpenCV::fourPointTransform(*input, warped, approx);
			
			cv::RotatedRect boundingBox = cv::minAreaRect(contours[i]);
			cv::Point2f corners[4];
			boundingBox.points(corners);

			double sideA = funcGeometry2D::calcDist2D(approx[1], approx[0]);
			double sideB = funcGeometry2D::calcDist2D(approx[1], approx[2]);
			double sideX = sideA / sideB;

			if (sideX >= 0.25 && sideX <= 4)
			{
				std::vector<cv::Point> cornersV;
				for (int j = 0; j < 4; j += 1)
					cornersV.push_back(corners[j]);

				funcOpenCV::fourPointTransform(*input, warped, cornersV);
				
				cv::Mat output;
				cv::Size size = input->size();
				cv::Size sizeW = warped.size();
				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);
				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);

				funcOpenCV::addMinimapRD(*input, warped, output, sizeMap, scanMapSize, scanMapColor);
				*input = output.clone();

				cv::drawContours(*input, contours, i, scanRectColor, scanRectSize);

				cv::line(*input, corners[0], corners[1], sectionColor, scanRectSize);
				cv::line(*input, corners[1], corners[2], sectionColor, scanRectSize);
				cv::line(*input, corners[2], corners[3], sectionColor, scanRectSize);
				cv::line(*input, corners[3], corners[0], sectionColor, scanRectSize);
			}
			break;
		}
	}
}