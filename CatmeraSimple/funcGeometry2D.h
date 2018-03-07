#pragma once
#ifndef FUNGEOMETRY2D_H
#define FUNGEOMETRY2D_H

#include <opencv2\opencv.hpp>

#include <math.h>
#include <vector>

namespace funcGeometry2D
{	
	double calcDist2D(cv::Point pixelA, cv::Point pixelB);

	bool checkAspectRatio2D(cv::Point pixleA, cv::Point pixelB, cv::Point pixelC, double ratio);
	
	//struct Line
	//{
	//	cv::Point pixelA;
	//	cv::Point pixelB;
	//};

	//typedef enum twolineState
	//{
	//	TWOLINESTATE_CROSS,
	//	TWOLINESTATE_COLLINEAR,
	//	TWOLINESTATE_PARALLEL,
	//} twolineState;

	//inline bool checkEqual(double x, double y);
	//twolineState calcIntersection2D(const Line &L1, const Line &L2, cv::Point &P);
	//int findMinSide(std::vector<cv::Point> &input);

	//int convertPoly5to4(std::vector<cv::Point> &input, std::vector<cv::Point> &output);
}

#endif // !FUNGEOMETRY2D_H
