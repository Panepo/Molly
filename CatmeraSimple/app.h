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
	app(std::string title);
	void process();

	// states
	void stateColor();
	void stateInfrared();
	void stateDepth();

	// events
	void eventKeyboard();
	static void eventMouseS(int event, int x, int y, int flags, void* userdata);
	void eventMouse(int event, int x, int y, int flags);

	// settings
	void setResolution(int stream, int width, int height, int fps);
	void setVisualPreset(std::string preset);

	// public parameters
	cv::Mat outputMat;

private:
	// realsense parameters
	rs2::pipeline pipeline;
	rs2::config config;
	rs2_intrinsics intrinsics;

	// application parameters
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
	double elapsed = 0;

	// stream post processing
	void streamPostProcess(cv::Mat* input, rs2::depth_frame* depth);
	
	// stream pointer and related parameters
	void streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin);
	float pixel[2] = { 0, 0 };
	float point[3] = { 0, 0, 0 };
	
	// stream infoer and related parameters
	void streamInfoer(cv::Mat* input, std::string text);
	
	// stream zoomer and related parameters
	cv::Mat streamZoomer(cv::Mat* input);
	int pixelZoom[2] = { 0, 0 };
	int roiZoom[2] = { 0, 0 };
	float scaleZoom = 1;
};


#endif // !APP_H