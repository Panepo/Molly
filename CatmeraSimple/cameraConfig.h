#pragma once
#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H

#include <librealsense2/rs.hpp>

#define EnableColor		1
#define EnableInfrared	2
#define EnableDepth		4

#define ColorWidth	1280
#define ColorHeight  720
#define ColorFPS	  30

#define DepthWidth	1280
#define DepthHeight  720
#define DepthFPS      30

#define VisualPreset "High Density"

static int detectDevice()
{
	int stream = 0;
	
	rs2::context ctx = rs2::context();
	rs2::device_list devices = ctx.query_devices();
	rs2::device selected_device;

	if (devices.size() == 0)
	{
		std::cerr << "No device connected, please connect a RealSense device" << std::endl;
		rs2::device_hub device_hub(ctx);
		selected_device = device_hub.wait_for_device();
	}
	else
	{
		std::string name = "Unknown Device";
		
		for (rs2::device device : devices)
		{
			if (device.supports(RS2_CAMERA_INFO_NAME))
				name = device.get_info(RS2_CAMERA_INFO_NAME);
			
			std::cout << name << std::endl;
		}

		if (name == "Intel RealSense 410")
			stream = EnableInfrared | EnableDepth;
		else if (name == "Intel RealSense 415")
			stream = EnableColor | EnableInfrared | EnableDepth;
		else if (name == "Intel RealSense 435")
			stream = EnableColor | EnableInfrared | EnableDepth;
	}

	return stream;
}

static std::string get_device_name(const rs2::device& dev)
{
	// Each device provides some information on itself, such as name:
	std::string name = "Unknown Device";
	if (dev.supports(RS2_CAMERA_INFO_NAME))
		name = dev.get_info(RS2_CAMERA_INFO_NAME);

	// and the serial number of the device:
	std::string sn = "########";
	if (dev.supports(RS2_CAMERA_INFO_SERIAL_NUMBER))
		sn = std::string("#") + dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);

	return name + " " + sn;
}

#endif // !CAMERACONFIG_H
