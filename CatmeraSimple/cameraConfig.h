#pragma once
#ifndef CAMERACONFIG_H
#define CAMERACONFIG_H

#include <librealsense2/rs.hpp>

#define WindowTitle "Molly Catmera"

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

			if (name == "Intel RealSense 410")
			{
				stream = EnableInfrared | EnableDepth;
				break;
			}
			else if (name == "Intel RealSense 415")
			{
				stream = EnableColor | EnableDepth;
				break;
			}
			else if (name == "Intel RealSense 435")
			{
				stream = EnableColor | EnableDepth;
				break;
			}
		}

		
	}

	return stream;
}

#endif // !CAMERACONFIG_H
