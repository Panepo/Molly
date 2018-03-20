#pragma once
#ifndef FUNGEOMETRY3D_H
#define FUNGEOMETRY3D_H

#include <opencv2\opencv.hpp>
#include <librealsense2\rs.hpp>
#include <librealsense2\rsutil.h>

#include <math.h>
#include <vector>

namespace funcGeometry3D
{
	float calcDist3D(float pixelA[2], float pixelB[2], const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
	double calcArea3D(std::vector<cv::Point> contours, cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, cv::Point location, cv::Scalar lineColor, int lineSize);
}

#endif // !FUNGEOMETRY3D_H