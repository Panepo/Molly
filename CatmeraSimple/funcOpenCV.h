#pragma once
#ifndef FUNCOPENCV_H
#define FUNCOPENCV_H

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

#include <math.h>
#include <vector>
#include <iostream>

#include "funcGeometry2D.h"

namespace funcOpenCV
{
	void addMinimapRD(cv::Mat input, cv::Mat minimap, cv::Mat &output, cv::Size &sizeMap, int border, cv::Scalar color);
	void addMinimapLD(cv::Mat input, cv::Mat minimap, cv::Mat &output, cv::Size &sizeMap, int border, cv::Scalar color);
	
	inline bool compareXCords(cv::Point pixelA, cv::Point pixelB);
	inline bool compareYCords(cv::Point pixelA, cv::Point pixelB);
	inline bool compareDistance(std::pair<cv::Point, cv::Point> pixelA, std::pair<cv::Point, cv::Point> pixelB);
	void orderPixels(std::vector<cv::Point> input, std::vector<cv::Point> &output);
	void fourPointTransform(cv::Mat input, cv::Mat &output, std::vector<cv::Point> pixels);

	bool contourSorter(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2);
}

#endif // !FUNCOPENCV_H
