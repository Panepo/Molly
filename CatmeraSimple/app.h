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
	APPSTATE_MEASURE,
} appState;

class app
{
public:
	app(std::string title);
	void cameraInitial();
	void cameraProcess();

	// settings
	void setResolution(int stream, int width, int height, int fps);
	void setVisualPreset(std::string preset);

	// public parameters
	int state;
	cv::Mat outputMat;

private:
	// realsense parameters
	rs2::pipeline pipeline;
	rs2::config config;
	rs2_intrinsics intrinsics;

	// application parameters
	int stream = APPSTATE_EXIT;
	int ColorWidth = 640;
	int ColorHeight = 480;
	int ColorFPS = 30;
	int DepthWidth = 640;
	int DepthHeight = 480;
	int DepthFPS = 30;
	std::string windowTitle = "RealSense Viewer";
	std::string visualPreset = "High Density";
	rs2::spatial_filter filterSpat;
	rs2::temporal_filter filterTemp;
	double elapsed = 0;

	// states
	void stateColor();
	void stateInfrared();
	void stateDepth();
	void stateMeasure();

	// events
	void eventKeyboard();
	static void eventMouseS(int event, int x, int y, int flags, void* userdata);
	void eventMouse(int event, int x, int y, int flags);

	// stream post processing
	void streamPostProcess(cv::Mat* input, rs2::depth_frame* depth);
	void measurePostProcess(cv::Mat* input, rs2::depth_frame* depth);
	
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

	// measure pointer and related parameters
	void measurePointer(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
	float measureDist(const rs2_intrinsics* intr, const rs2::depth_frame* frame, float pixelA[3], float pixelB[3]);
	float pixelA[2] = { 0, 0 };
	float pixelB[2] = { 0, 0 };
	std::string distText;
};


#endif // !APP_H