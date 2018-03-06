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
	float calcDist3D(float pixelA[3], float pixelB[3], const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
}

#endif // !FUNGEOMETRY3D_H