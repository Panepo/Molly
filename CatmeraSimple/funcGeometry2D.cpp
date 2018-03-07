#include "stdafx.h"
#include "funcGeometry2D.h"

namespace funcGeometry2D
{
	double calcDist2D(cv::Point pixelA, cv::Point pixelB)
	{
		return sqrt(((pixelA.x - pixelB.x) * (pixelA.x - pixelB.x)) +
			((pixelA.y - pixelB.y) * (pixelA.y - pixelB.y)));
	}

	bool checkAspectRatio2D(cv::Point pixleA, cv::Point pixelB, cv::Point pixelC, double ratio)
	{
		double sideA = calcDist2D(pixleA, pixelB);
		double sideB = calcDist2D(pixleA, pixelC);
		double sideX = sideA / sideB;
		double ratioInv = 1 / ratio;
		
		if (sideX >= ratioInv && sideX <= ratio)
			return true;
		
		return false;
	}
	
	
	//inline bool checkEqual(double x, double y)
	//{
	//	return (fabs(x - y) < 1e-5);
	//}

	//twolineState calcIntersection2D(const Line &line1, const Line &line2, cv::Point &output)
	//{
	//	double A1 = line1.pixelB.y - line1.pixelA.y;
	//	double B1 = line1.pixelA.x - line1.pixelB.x;
	//	double C1 = line1.pixelB.x * line1.pixelA.y - line1.pixelA.x * line1.pixelB.y;
	//	double A2 = line2.pixelB.y - line2.pixelA.y;
	//	double B2 = line2.pixelA.x - line2.pixelB.x;
	//	double C2 = line2.pixelB.x * line2.pixelA.y - line2.pixelA.x * line2.pixelB.y;

	//	if (checkEqual(A1 * B2, B1 * A2)) {
	//		if (checkEqual((A1 + B1) * C2, (A2 + B2) * C1)) {
	//			return TWOLINESTATE_COLLINEAR;
	//		}
	//		else {
	//			return TWOLINESTATE_PARALLEL;
	//		}
	//	}
	//	else {
	//		//double xpos = (B2 * C1 - B1 * C2) / (A2 * B1 - A1 * B2);
	//		//double ypos = (A1 * C2 - A2 * C1) / (A2 * B1 - A1 * B2);
	//		
	//		output.x = (int)floor((B2 * C1 - B1 * C2) / (A2 * B1 - A1 * B2));
	//		output.y = (int)floor((A1 * C2 - A2 * C1) / (A2 * B1 - A1 * B2));
	//		return TWOLINESTATE_CROSS;
	//	}
	//}

	//int findMinSide(std::vector<cv::Point> &input)
	//{
	//	int pairIdx = 0;
	//	for (int j = 0; j < (int)input.size(); j += 1)
	//	{
	//		double length, pairMin;
	//		if (j == 0)
	//		{
	//			length = sqrt(pow(input[j].x - input[(int)input.size()].x, 2) +
	//				pow(input[j].y - input[(int)input.size()].y, 2));
	//			pairMin = length;
	//		}
	//		else
	//		{
	//			length = sqrt(pow(input[j].x - input[j - 1].x, 2) +
	//				pow(input[j].y - input[j - 1].y, 2));

	//			if (length < pairMin)
	//			{
	//				pairMin = length;
	//				pairIdx = j;
	//			}
	//		}
	//	}

	//	return pairIdx;
	//}

	//int convertPoly5to4(std::vector<cv::Point> &input, std::vector<cv::Point> &output)
	//{
	//	if (input.size() != 5 || output.size() != 0)
	//		return false;
	//	
	//	int pairIdx = findMinSide(input);

	//	Line lineA, lineB;
	//	if (pairIdx >= 2)
	//	{
	//		lineA = { input[pairIdx], input[pairIdx + 1] };
	//		lineB = { input[pairIdx - 1], input[pairIdx - 2] };
	//	}
	//	else if (pairIdx == 1)
	//	{
	//		lineA = { input[pairIdx], input[pairIdx + 1] };
	//		lineB = { input[pairIdx - 1], input[(int)input.size()] };
	//	}
	//	else
	//	{
	//		lineA = { input[pairIdx], input[(int)input.size()] };
	//		lineB = { input[(int)input.size()], input[(int)input.size() - 1] };
	//	}
	//	cv::Point result;
	//	int lineState = calcIntersection2D(lineA, lineB, result);

	//	if (lineState == TWOLINESTATE_CROSS)
	//	{
	//		for (int j = 0; j < (int)input.size(); j += 1)
	//		{
	//			if (pairIdx == 0)
	//			{
	//				if (j == (int)input.size())
	//					continue;
	//				else if (j == pairIdx)
	//					output.push_back(result);
	//				else
	//					output.push_back(input[j]);
	//			}
	//			else
	//			{
	//				if (j == pairIdx - 1)
	//					continue;
	//				else if (j == pairIdx)
	//					output.push_back(result);
	//				else
	//					output.push_back(input[j]);
	//			}
	//		}
	//		return true;
	//	}
	//	else
	//		return false;
	//}
}