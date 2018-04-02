#include "stdafx.h"
#include "app.h"
#include "funcGeometry2D.h"
#include "funcGeometry3D.h"
#include "funcOpenCV.h"

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

void app::scannerProcess(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
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


