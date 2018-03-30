#include "stdafx.h"
#include "app.h"
#include "funcGeometry2D.h"
#include "funcGeometry3D.h"
#include "funcOpenCV.h"

#define	ENABLE_GAZE	0

// =================================================================================
// Application minor private functions for scanner
// =================================================================================

void app::scannerDrawer(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	cv::Size size = input->size();
	
	// =================================================================================
	// TODO:: optimize scanner pre processing 
	// =================================================================================
	cv::Mat inputEdge;
	funcOpenCV::cannyBlur(*input, inputEdge, 50, 150);

	// =================================================================================
	// =================================================================================

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), funcOpenCV::contourSorter);

	std::vector<cv::Point> approx;
	for (int i = 0; i < (int)contours.size(); i += 1)
	{
		if (cv::contourArea(contours[i]) <= scanMinArea)
			break;
		
		cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);
		
		if ((int)approx.size() == 4)
		{			
			if (funcGeometry2D::checkAspectRatio2D(approx[1], approx[0], approx[2], 4))
			{
				cv::Mat warped;
				funcOpenCV::fourPointTransform(*input, warped, approx);

				float pixelA[2] = { approx[1].x, approx[1].y };
				float pixelB[2] = { approx[0].x, approx[0].y };
				float pixelC[2] = { approx[2].x, approx[2].y };

				float distA = funcGeometry3D::calcDist3D(pixelA, pixelB, depth, intrin);
				float distB = funcGeometry3D::calcDist3D(pixelA, pixelC, depth, intrin);

				cv::Size sizeW = warped.size();
				float mod;
				if (distA > distB)
				{
					if (sizeW.width > sizeW.height)
					{
						mod = (float)sizeW.height * distA / distB;
						cv::resize(warped, warped, cv::Size((int)mod, sizeW.height), 0, 0, CV_INTER_LINEAR);
					}
					else
					{
						mod = (float)sizeW.width * distA / distB;
						cv::resize(warped, warped, cv::Size(sizeW.height, (int)mod), 0, 0, CV_INTER_LINEAR);
					}
				}
				else
				{
					if (sizeW.width > sizeW.height)
					{
						mod = (float)sizeW.height * distB / distA;
						cv::resize(warped, warped, cv::Size((int)mod, sizeW.height), 0, 0, CV_INTER_LINEAR);
					}
					else
					{
						mod = (float)sizeW.width * distB / distA;
						cv::resize(warped, warped, cv::Size(sizeW.height, (int)mod), 0, 0, CV_INTER_LINEAR);
					}
				}
				sizeW = warped.size();

				//cv::Size sizeW = warped.size();
				
				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);

				if (heightMod > size.height - 100)
					heightMod = (float)(size.height - 100);

				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);
				cv::Mat overlay;
				funcOpenCV::addMinimapRD(*input, warped, overlay, sizeMap, scanMapSize, scanMapColor);
				*input = overlay.clone();
				cv::drawContours(overlay, contours, i, scanRectColor, scanRectSize);
				cv::addWeighted(overlay, transparentP, *input, transparentO, 0, *input);
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

			if (funcGeometry2D::checkAspectRatio2D(corners[1], corners[0], corners[2], 4))
			{
				std::vector<cv::Point> cornersV;
				for (int j = 0; j < 4; j += 1)
					cornersV.push_back(corners[j]);

				funcOpenCV::fourPointTransform(*input, warped, cornersV);
				
				float pixelA[2] = { approx[1].x, approx[1].y };
				float pixelB[2] = { approx[0].x, approx[0].y };
				float pixelC[2] = { approx[2].x, approx[2].y };

				float distA = funcGeometry3D::calcDist3D(pixelA, pixelB, depth, intrin);
				float distB = funcGeometry3D::calcDist3D(pixelA, pixelC, depth, intrin);

				cv::Size sizeW = warped.size();
				float mod;
				if (distA > distB)
				{
					if (sizeW.width > sizeW.height)
					{
						mod = (float)sizeW.height * distA / distB;
						cv::resize(warped, warped, cv::Size((int)mod, sizeW.height), 0, 0, CV_INTER_LINEAR);
					}
					else
					{
						mod = (float)sizeW.width * distA / distB;
						cv::resize(warped, warped, cv::Size(sizeW.height, (int)mod), 0, 0, CV_INTER_LINEAR);
					}
				}
				else
				{
					if (sizeW.width > sizeW.height)
					{
						mod = (float)sizeW.height * distB / distA;
						cv::resize(warped, warped, cv::Size((int)mod, sizeW.height), 0, 0, CV_INTER_LINEAR);
					}
					else
					{
						mod = (float)sizeW.width * distB / distA;
						cv::resize(warped, warped, cv::Size(sizeW.height, (int)mod), 0, 0, CV_INTER_LINEAR);
					}
				}
				//cv::Size 
					
				sizeW = warped.size();
				
				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);

				if (heightMod > size.height - 100)
					heightMod = (float)(size.height - 100);
				
				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);
				cv::Mat overlay;
				funcOpenCV::addMinimapRD(*input, warped, overlay, sizeMap, scanMapSize, scanMapColor);
				*input = overlay.clone();

				cv::drawContours(overlay, contours, i, scanRectColor, scanRectSize);

				cv::line(overlay, corners[0], corners[1], sectionColor, scanRectSize);
				cv::line(overlay, corners[1], corners[2], sectionColor, scanRectSize);
				cv::line(overlay, corners[2], corners[3], sectionColor, scanRectSize);
				cv::line(overlay, corners[3], corners[0], sectionColor, scanRectSize);

				cv::addWeighted(overlay, transparentP, *input, transparentO, 0, *input);
			}
			break;
		}	
	}
}

void app::scannerDrawerSharp(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	cv::Size size = input->size();

	cv::Mat inputGray;
	cv::cvtColor(*input, inputGray, CV_RGB2GRAY);

	// =================================================================================
	// TODO:: optimize scanner pre processing 
	// =================================================================================

	//cv::threshold(inputGray, inputGray, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
	/*cv::Mat structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4));
	morphologyEx(inputGray, inputGray, cv::MORPH_OPEN, structuringElmt);
	morphologyEx(inputGray, inputGray, cv::MORPH_CLOSE, structuringElmt);
	cv::GaussianBlur(inputGray, inputGray, cv::Size(7, 7), 0, 0);*/
	//cv::equalizeHist(inputGray, inputGray);

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(4);
	clahe->apply(inputGray, inputGray);

	cv::Mat inputEdge;
	cv::Canny(inputGray, inputEdge, 50, 25, 3);
	

	// =================================================================================
	// =================================================================================

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), funcOpenCV::contourSorter);

	std::vector<cv::Point> approx;
	for (int i = 0; i < (int)contours.size(); i += 1)
	{
		if (cv::contourArea(contours[i]) <= scanMinArea)
			break;

		cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);

		if ((int)approx.size() == 4)
		{
			if (funcGeometry2D::checkAspectRatio2D(approx[1], approx[0], approx[2], 4))
			{
				cv::Mat warped;
				funcOpenCV::fourPointTransform(*input, warped, approx);


				cv::Size sizeW = warped.size();

				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);

				if (heightMod > size.height - 100)
					heightMod = (float)(size.height - 100);

				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);
				cv::Mat output;
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

			if (funcGeometry2D::checkAspectRatio2D(corners[1], corners[0], corners[2], 4))
			{
				std::vector<cv::Point> cornersV;
				for (int j = 0; j < 4; j += 1)
					cornersV.push_back(corners[j]);

				funcOpenCV::fourPointTransform(*input, warped, cornersV);

				cv::Size sizeW = warped.size();

				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);

				if (heightMod > size.height - 100)
					heightMod = (float)(size.height - 100);

				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);
				cv::Mat output;
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

void app::scannerDrawerGaze(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	cv::Size size = input->size();

	cv::Mat inputGray;
	cv::cvtColor(*input, inputGray, CV_RGB2GRAY);

	// =================================================================================
	// TODO:: optimize scanner pre processing 
	// =================================================================================

#if ENABLE_GAZE
	//cv::GaussianBlur(inputGray, inputGray, cv::Size(7, 7), 0, 0);
	//cv::equalizeHist(inputGray, inputGray);

	cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(4);
	clahe->apply(inputGray, inputGray);

	cv::Rect gazeRoi;
	gazeRoi.x = (int)(size.width / 2 - scanGazeRect);
	gazeRoi.y = (int)(size.height / 2 - scanGazeRect);
	gazeRoi.width = scanGazeRect * 2;
	gazeRoi.height = scanGazeRect * 2;
	cv::Mat gaze = inputGray(gazeRoi);

	double gazeMin, gazeMax;
	cv::minMaxLoc(gaze, &gazeMin, &gazeMax);

	cv::threshold(inputGray, inputGray, gazeMin, 255, CV_THRESH_BINARY);
	cv::imshow("test", inputGray);

#else
	//cv::threshold(inputGray, inputGray, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
	cv::Mat structuringElmt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(4, 4));
	morphologyEx(inputGray, inputGray, cv::MORPH_OPEN, structuringElmt);
	morphologyEx(inputGray, inputGray, cv::MORPH_CLOSE, structuringElmt);
	cv::GaussianBlur(inputGray, inputGray, cv::Size(7, 7), 0, 0);
	//cv::equalizeHist(inputGray, inputGray);

	/*cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
	clahe->setClipLimit(4);
	clahe->apply(inputGray, inputGray);*/

#endif

	cv::Mat inputEdge;
	cv::Canny(inputGray, inputEdge, 50, 25, 3);
	//cv::imshow("test", inputEdge);

	// =================================================================================
	// =================================================================================

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), funcOpenCV::contourSorter);

	std::vector<cv::Point> approx;
	for (int i = 0; i < (int)contours.size(); i += 1)
	{
		if (cv::contourArea(contours[i]) <= scanMinArea)
			break;

		cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);

		if ((int)approx.size() == 4)
		{
			//funcOpenCV::orderPixels(approx, approx);

			if (funcGeometry2D::checkAspectRatio2D(approx[1], approx[0], approx[2], 4))
			{
				cv::Mat warped;
				funcOpenCV::fourPointTransform(*input, warped, approx);


				cv::Size sizeW = warped.size();

				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);

				if (heightMod > size.height - 100)
					heightMod = (float)(size.height - 100);

				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);
				//cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)(size.height / 4));
				cv::Mat output;
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
			//funcOpenCV::fourPointTransform(*input, warped, approx);

			cv::RotatedRect boundingBox = cv::minAreaRect(contours[i]);
			cv::Point2f corners[4];
			boundingBox.points(corners);

			if (funcGeometry2D::checkAspectRatio2D(corners[1], corners[0], corners[2], 4))
			{
				std::vector<cv::Point> cornersV;
				for (int j = 0; j < 4; j += 1)
					cornersV.push_back(corners[j]);

				funcOpenCV::fourPointTransform(*input, warped, cornersV);

				cv::Size sizeW = warped.size();

				float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);

				if (heightMod > size.height - 100)
					heightMod = (float)(size.height - 100);

				cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);
				//cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)(size.height / 4));
				cv::Mat output;
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

#if ENABLE_GAZE
	// draw gaze point
	cv::rectangle(*input, gazeRoi, scanGazeColor, scanGazeSize);
#endif
}

void app::scannerEventHandler(int event, int x, int y, int flags)
{
	float value;
	
	switch (event)
	{
	case CV_EVENT_MOUSEWHEEL:
		pixelZoom[0] = x;
		pixelZoom[1] = y;

		value = (float)cv::getMouseWheelDelta(flags);
		//std::cout << value << std::endl;
		if (value > 0 && scaleZoom < zoomerScaleMax)
			scaleZoom += (float) 0.1;
		else if (value < 0 && scaleZoom > zoomerScaleMin)
			scaleZoom -= (float) 0.1;
		break;
	default:
		break;
	}
}


