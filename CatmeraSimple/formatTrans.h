#pragma once
#ifndef FORMATTRANS_H
#define FORMATTRANS_H

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

class formatTrans
{
public:
	static cv::Mat frame2Mat(const rs2::frame& f);
};
#endif // !FORMATTRANS_H