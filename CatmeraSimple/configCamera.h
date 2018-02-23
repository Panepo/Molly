#pragma once
#ifndef CONFIGCAMERA_H
#define CONFIGCAMERA_H

#include <librealsense2/rs.hpp>

#define EnableColor		1
#define EnableInfrared	2

static int detectDevice()
{
	rs2::context ctx = rs2::context();
	rs2::device_list devices = ctx.query_devices();
	rs2::device selected_device;

	if (devices.size() == 0)
	{
		throw std::runtime_error("No device connected, please connect a RealSense device");
		//rs2::device_hub device_hub(ctx);
		//selected_device = device_hub.wait_for_device();
	}
	else
	{
		std::string name = "Unknown Device";
		
		for (rs2::device device : devices)
		{
			if (device.supports(RS2_CAMERA_INFO_NAME))
				name = device.get_info(RS2_CAMERA_INFO_NAME);
			
			std::cout << "Detected device: " << name << std::endl;

			if (name == "Intel RealSense 410")
				return EnableInfrared;
			else if (name == "Intel RealSense 415")
				return EnableColor;
			else if (name == "Intel RealSense 435")
				return EnableColor;
		}
		throw std::runtime_error("No device connected, please connect a RealSense device");
	}
}

#endif // !CONFIGCAMERA_H
