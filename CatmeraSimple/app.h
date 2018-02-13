#pragma once
#ifndef APP_H
#define APP_H

#include <librealsense2\rs.hpp>
#include <opencv2\opencv.hpp>
#include <librealsense2\rsutil.h>

#include "configCamera.h"
#include "configOpenCV.h"
#include "funcFormat.h"

typedef enum appState
{
	APPSTATE_EXIT,
	APPSTATE_COLOR,
	APPSTATE_INFRARED,
	APPSTATE_DEPTH,
} appState;

class app
{
public:
	int state;
	app();
	void stateColor();
	void stateInfrared();
	void stateDepth();

	void stateSwitch();

private:
	rs2::pipeline pipeline;
	rs2::config config;
	rs2_intrinsics intrinsics;
	
	float pixel[2];
	float point[3];
	int stream;
	
	
	void streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin);
	
	static void getCoordPixelS(int event, int x, int y, int flags, void* userdata);
	void getCoordPixel(int event, int x, int y, int flags);
};


#endif // !APP_H