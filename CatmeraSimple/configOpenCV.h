#pragma once
#ifndef CONFIGOPENCV_H
#define CONFIGOPENCV_H

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
#define sectionMapColor  cv::Scalar(0, 255,0)		// green

#endif // !CONFIGOPENCV_H

