#pragma once
#ifndef FUNCFORMAT_H
#define FUNCFORMAT_H

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

class funcFormat
{
public:
	static cv::Mat frame2Mat(const rs2::frame& f);
};
#endif // !FUNCFORMAT_H