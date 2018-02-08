#pragma once
#ifndef REALSENSE_H
#define REALSENSE_H

#include <librealsense2/rs.hpp>


class realsense {
public:
	realsense();

	void setRes(int steam, int width, int height);


	void startStream();
	void getFrame();

	rs2::frame colorFrame;
	//rs2::frame infraredFrame;
	rs2::frame depthFrame;
	rs2::frame depthColorMap;

private:
	rs2::pipeline pipeline;
	rs2::config config;
	rs2::align align;

	rs2::colorizer colorMap;

	int colorWidth = 640;
	int colorHeight = 480;
	int infWidth = 640;
	int infHeight = 480;
	int depthWidth = 640;
	int depthHeight = 480;
};


#endif // REALSENSE_H