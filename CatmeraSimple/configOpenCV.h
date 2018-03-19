#pragma once
#ifndef CONFIGOPENCV_H
#define CONFIGOPENCV_H

#define transparentP	0.4
#define transparentO	(1 - transparentP)

#define pointerSize		5
#define pointerColor	cv::Scalar(255, 255, 255)	// white
#define pointerFontA	cv::FONT_HERSHEY_DUPLEX
#define pointerFontB	cv::FONT_HERSHEY_SIMPLEX
#define pointerColorFA	cv::Scalar(0, 0, 0)			// black
#define pointerColorFB	cv::Scalar(0, 255,255)		// yellow

#define inforerFont		cv::FONT_HERSHEY_TRIPLEX
#define inforerColor	cv::Scalar(255,255,0)		// teal

#define zoomerScaleMin	0.1
#define zoomerScaleMax	1
#define zoomerLineSize	5
#define zoomerLineColor cv::Scalar(0, 255,255)		// yellow
#define zoomerMapSize	3
#define zoomerMapColor  cv::Scalar(0, 255,0)		// green

#define measureSize		10
#define measureColor	cv::Scalar(0, 255,0)		// green

#define sectionHeight	720
#define sectionPreSize	(sectionHeight / 10)
#define sectionColor	cv::Scalar(0, 255,255)		// yellow
#define sectionBGColor	cv::Scalar(0, 0, 0)			// black
#define sectionMapSize	3
#define sectionMapColor cv::Scalar(0, 255,0)		// green

#define scanRectColor	cv::Scalar(0, 255,0)		// green
#define scanRectSize	2
#define scanMinArea		10000
#define scanMapSize		3
#define scanMapColor	cv::Scalar(0, 255,0)		// green
#define scanGazeColor	cv::Scalar(0, 255,255)		// yellow
#define scanGazeSize	1
#define scanGazeRect	5

#define measRectColor1	cv::Scalar(0, 255,255)		// yellow
#define measRectSize1	1
#define measRectColor2	cv::Scalar(0, 255,0)		// green
#define measRectSize2	2
#define measContColor	cv::Scalar(255,255,0)		// teal
#define measContSize	2
#define measApproxMin	4
#define measApproxMax	10
#define measLineColor	cv::Scalar(0, 255,255)		// yellow
#define measLineSize	2

#endif // !CONFIGOPENCV_H

