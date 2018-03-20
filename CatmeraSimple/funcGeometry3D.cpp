#include "stdafx.h"
#include "funcGeometry3D.h"

namespace funcGeometry3D
{
	float calcDist3D(float pixelA[2], float pixelB[2], const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
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

	double calcArea3D(std::vector<cv::Point> contours, cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, cv::Point location, cv::Scalar lineColor, int lineSize)
	{
		cv::RotatedRect boundingBox = cv::minAreaRect(contours);
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

		cv::line(*input, cv::Point(a1[0], a1[1]), cv::Point(a2[0], a2[1]), lineColor, lineSize);
		cv::line(*input, cv::Point(b1[0], b1[1]), cv::Point(b2[0], b2[1]), lineColor, lineSize);

		a1[0] += location.x;
		a1[1] += location.y;
		a2[0] += location.x;
		a2[1] += location.y;
		b1[0] += location.x;
		b1[1] += location.y;
		b2[0] += location.x;
		b2[1] += location.y;

		float distA = calcDist3D(a1, a2, depth, intrin);
		float distB = calcDist3D(b1, b2, depth, intrin);
		
		std::vector<cv::Point> contourBox;
		for (int i = 0; i < 4; i += 1)
			contourBox.push_back(corners[i]);

		double areaA = cv::contourArea(contours);
		double areaB = cv::contourArea(contourBox);
		
		double area = distA * distB * areaA / areaB;

		return floor(area * 100) / 100;
		
	}
}
