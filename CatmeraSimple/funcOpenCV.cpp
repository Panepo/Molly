#include "stdafx.h"
#include "funcOpenCV.h"

namespace funcOpenCV
{
	void addMinimapRD(cv::Mat input, cv::Mat minimap, cv::Mat &output, cv::Size &sizeMap, int border, cv::Scalar color)
	{
		cv::Size size = input.size();
		cv::resize(minimap, minimap, sizeMap, 0, 0, CV_INTER_LINEAR);
		cv::copyMakeBorder(minimap, minimap, border, border, border, border, cv::BORDER_CONSTANT, color);
		minimap.copyTo(input(cv::Rect(size.width - minimap.cols - 10, size.height - minimap.rows - 10,
			minimap.cols, minimap.rows)));
		output = input.clone();
	}

	void addMinimapLD(cv::Mat input, cv::Mat minimap, cv::Mat &output, cv::Size &sizeMap, int border, cv::Scalar color)
	{
		cv::Size size = input.size();
		cv::resize(minimap, minimap, sizeMap, 0, 0, CV_INTER_LINEAR);
		cv::copyMakeBorder(minimap, minimap, border, border, border, border, cv::BORDER_CONSTANT, color);
		minimap.copyTo(input(cv::Rect(10, size.height - minimap.rows - 10,
			minimap.cols, minimap.rows)));
		output = input.clone();
	}
	
	inline bool compareXCords(cv::Point pixelA, cv::Point pixelB)
	{
		return (pixelA.x < pixelB.x);
	}

	inline bool compareYCords(cv::Point pixelA, cv::Point pixelB)
	{
		return (pixelA.y < pixelB.y);
	}

	inline bool compareDistance(std::pair<cv::Point, cv::Point> pixelA, std::pair<cv::Point, cv::Point> pixelB)
	{
		return (cv::norm(pixelA.first - pixelA.second) < cv::norm(pixelB.first - pixelB.second));
	}
	
	void orderPixels(std::vector<cv::Point> input, std::vector<cv::Point> &output)
	{
		std::sort(input.begin(), input.end(), compareXCords);
		std::vector<cv::Point> lm(input.begin(), input.begin() + 2);
		std::vector<cv::Point> rm(input.end() - 2, input.end());

		std::sort(lm.begin(), lm.end(), compareYCords);
		cv::Point tl(lm[0]);
		cv::Point bl(lm[1]);
		std::vector<std::pair<cv::Point, cv::Point>> temp;
		for (size_t i = 0; i< rm.size(); i++)
		{
			temp.push_back(std::make_pair(tl, rm[i]));
		}

		std::sort(temp.begin(), temp.end(), compareDistance);
		cv::Point tr(temp[0].second);
		cv::Point br(temp[1].second);

		output.push_back(tl);
		output.push_back(tr);
		output.push_back(br);
		output.push_back(bl);
	}


	void fourPointTransform(cv::Mat input, cv::Mat &output, std::vector<cv::Point> pixels)
	{
		std::vector<cv::Point> orderedPixels;
		orderPixels(pixels, orderedPixels);

		double widthA = funcGeometry2D::calcDist2D(orderedPixels[2], orderedPixels[3]);
		double widthB = funcGeometry2D::calcDist2D(orderedPixels[1], orderedPixels[0]);
		double widthMax = std::max(widthA, widthB);

		double heightA = funcGeometry2D::calcDist2D(orderedPixels[1], orderedPixels[2]);
		double heightB = funcGeometry2D::calcDist2D(orderedPixels[0], orderedPixels[3]);
		double heightMax = std::max(heightA, heightB);

		cv::Point2f rectOrig[] =
		{
			cv::Point2f(orderedPixels[0]),
			cv::Point2f(orderedPixels[1]),
			cv::Point2f(orderedPixels[2]),
			cv::Point2f(orderedPixels[3]),
		};
		cv::Point2f rectOutput[] =
		{
			cv::Point2f(0, 0),
			cv::Point2f((float)widthMax - 1, 0),
			cv::Point2f((float)widthMax - 1, (float)heightMax - 1),
			cv::Point2f(0, (float)heightMax - 1)
		};
		cv::Mat matrix = cv::getPerspectiveTransform(rectOrig, rectOutput);
		cv::warpPerspective(input, output, matrix, cv::Size((int)widthMax, (int)heightMax));
	}
}