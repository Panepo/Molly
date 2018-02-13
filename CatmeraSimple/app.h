#pragma once
#ifndef APP_H
#define APP_H

#include <librealsense2\rs.hpp>
#include <opencv2\opencv.hpp>
#include <librealsense2\rsutil.h>

#include <ctime>
#include <sstream>

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
	void process();

	void stateColor();
	void stateInfrared();
	void stateDepth();

	void kbCommand();

	void setResolution(int stream, int width, int height, int fps);
	void setWindowTitle(std::string title);
	void setVisualPreset(std::string preset);

private:
	rs2::pipeline pipeline;
	rs2::config config;
	rs2_intrinsics intrinsics;

	int stream = APPSTATE_EXIT;
	int ColorWidth = 1280;
	int ColorHeight = 720;
	int ColorFPS = 30;
	int DepthWidth = 1280;
	int DepthHeight = 720;
	int DepthFPS = 30;
	std::string windowTitle = "RealSense Viewer";
	std::string visualPreset = "High Density";

	rs2::spatial_filter spat_filter;
	rs2::temporal_filter temp_filter;
	
	float pixel[2] = {0, 0};
	float point[3] = {0, 0, 0};

	double elapsed = 0;
	
	void streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin);
	void streamInfoer(cv::Mat* input, std::string text);
	
	static void getCoordPixelS(int event, int x, int y, int flags, void* userdata);
	void getCoordPixel(int event, int x, int y, int flags);
};


#endif // !APP_H