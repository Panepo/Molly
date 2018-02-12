#pragma once
#ifndef APP_H
#define APP_H

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

#include "cameraConfig.h"
#include "formatTrans.h"

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
	static void stateColor(rs2::pipeline pipeline, rs2::colorizer colorize);
	static void stateInfrared(rs2::pipeline pipeline, rs2::colorizer colorize);
	static void stateDepth(rs2::pipeline pipeline, rs2::colorizer colorize, int stream);

	void stateSwitch(int stream);
};


#endif // !APP_H