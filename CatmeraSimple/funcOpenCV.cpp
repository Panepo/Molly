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

	bool contourSorter(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2)
	{
		double i = fabs(cv::contourArea(cv::Mat(contour1)));
		double j = fabs(cv::contourArea(cv::Mat(contour2)));
		return (i > j);
	}
	
	void cannyBlur(cv::Mat &input, cv::Mat &output, double threshold1, double threshold2)
	{
		cv::Mat inputGray;
		cv::cvtColor(input, inputGray, CV_RGB2GRAY);
		
		cv::Mat structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4));
		morphologyEx(inputGray, inputGray, cv::MORPH_OPEN, structuringElmt);
		morphologyEx(inputGray, inputGray, cv::MORPH_CLOSE, structuringElmt);
		cv::GaussianBlur(inputGray, inputGray, cv::Size(7, 7), 0, 0);

		cv::Mat inputEdge;
		cv::Canny(inputGray, output, threshold1, threshold2, 3);

		//cv::imshow("canny test", output);
	}
	
	void cannySharp(cv::Mat & input, cv::Mat & output, double threshold1, double threshold2)
	{
		cv::Mat inputGray;
		cv::cvtColor(input, inputGray, CV_RGB2GRAY);

		cv::Mat structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
		morphologyEx(inputGray, inputGray, cv::MORPH_OPEN, structuringElmt);
		morphologyEx(inputGray, inputGray, cv::MORPH_CLOSE, structuringElmt);
		cv::GaussianBlur(inputGray, inputGray, cv::Size(5, 5), 0, 0);

		cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
		clahe->setClipLimit(4);
		clahe->apply(inputGray, inputGray);

		cv::Mat inputEdge;
		cv::Canny(inputGray, output, threshold1, threshold2, 3);

		cv::imshow("canny test", output);
	}
}