#pragma once
#ifndef APP_H
#define APP_H

#include <librealsense2\rs.hpp>
#include <opencv2\opencv.hpp>
//#include <boost/range/irange.hpp>

#include <omp.h>

//#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm> 

#include "configCamera.h"
#include "configOpenCV.h"
#include "funcFormat.h"

typedef enum appState
{
	APPSTATE_EXIT,
	APPSTATE_COLOR,
	APPSTATE_INFRARED,
	APPSTATE_DEPTH,
	APPSTATE_RULER,
	APPSTATE_PHOTOGRAPHER,
	APPSTATE_SCANNER,
	APPSTATE_MEASURER,
} appState;

typedef enum rulerState
{
	RULER_WAIT,
	RULER_PAINT,
	RULER_LINE,
} rulerState;

typedef enum measurerState
{
	MEASURER_WAIT,
	MEASURER_PAINT,
	MEASURER_RECT,
} measurerState;

class app
{
public:
	app(std::string title);
	void cameraInitial();
	void cameraProcess();

	// config settings
	void setResolution(int stream, int width, int height, int fps);
	void setVisualPreset(std::string preset);

	// public parameters
	int stream = APPSTATE_EXIT;
	appState state = APPSTATE_EXIT;
	cv::Mat outputMat;

	//void eventMouseCV(int event, int x, int y, int flags, int wheel);
	float pixel[2] = { 0, 0 };
	int pixelZoom[2] = { 0, 0 };
	float scaleZoom = 1;


private:
	// realsense parameters
	rs2::pipeline pipeline;
	rs2::config config;
	rs2_intrinsics intrinsics;

	// application parameters
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
	double elapsedAvg = 0;

	// states
	void stateColor();
	void stateInfrared();
	void stateDepth();
	void stateRuler();
	void statePhotographer();
	void stateScanner();
	void stateMeasurer();

	// events
	//void eventKeyboard();
	static void eventMouseS(int event, int x, int y, int flags, void* userdata);
	void eventMouse(int event, int x, int y, int flags);
	

	// stream post processing
	void postStreamer(cv::Mat* input, rs2::depth_frame* depth);
	void postRuler(cv::Mat* input, rs2::depth_frame* depth);
	void postPhotographer(cv::Mat* input, cv::Mat * depth);
	void postScanner(cv::Mat* input, rs2::depth_frame* depth);
	void postMeasurer(cv::Mat* input, rs2::depth_frame* depth);
	
	// stream pointer and related parameters
	void streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin, float point[3]);
	
	// stream infoer and related parameters
	void streamInfoer(cv::Mat* input, std::string text);
	
	// stream zoomer and related parameters
	cv::Mat streamZoomer(cv::Mat* input);
	int roiZoom[2] = { 0, 0 };
	

	// ruler pointer, drawer and related parameters
	rulerState rstate = RULER_WAIT;
	void rulerMain(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, rulerState& rstate);
	void rulerPaint(cv::Mat* input);
	void rulerPointer(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
	void rulerDrawer(cv::Mat* input, const rs2::depth_frame* depth);
	cv::Point pixelRulerA = { 0, 0 };
	cv::Point pixelRulerB = { 0, 0 };
	std::string distText;

	// align renderer and related parameters
	void photographerRenderer(cv::Mat* input, cv::Mat * depth);

	void scannerDrawer(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
	void scannerDrawerSharp(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
	void scannerDrawerGaze(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin);

	measurerState mstate = MEASURER_WAIT;
	void measurerMain(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, measurerState& mstate);
	void measurerPaint(cv::Mat* input);
	void measurerRect(cv::Mat * input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin);
	cv::Point pixelMeasureA = { 0, 0 };
	cv::Point pixelMeasureB = { 0, 0 };
};


#endif // !APP_H