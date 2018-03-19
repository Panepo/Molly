#include "stdafx.h"
#include "funcGeometry3D.h"

namespace funcGeometry3D
{
	float calcDist3D(float pixelA[2], float pixelB[2], const rs2::depth_frame* depth, const rs2_intrinsics* intrin)
	{
		float pointA[3] = { 0, 0, 0 };
		float pointB[3] = { 0, 0, 0 };

		float distA = depth->get_distance((int)pixelA[0], (int)pixelA[1]);
		float distB = depth->get_distance((int)pixelB[0], (int)pixelB[1]);

		rs2_deproject_pixel_to_point(pointA, intrin, pixelA, distA);
		rs2_deproject_pixel_to_point(pointB, intrin, pixelB, distB);

		return floor(sqrt(pow(pointA[0] - pointB[0], 2) +
			pow(pointA[1] - pointB[1], 2) +
			pow(pointA[2] - pointB[2], 2)) * 10000) / 100;
	}
}
