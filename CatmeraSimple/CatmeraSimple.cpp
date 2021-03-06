// CatmeraSimple.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <iostream>
#include <thread>

#include "app.h"

struct mouseState
{
	bool trigger = false;
	int event;
	int x;
	int y;
	int flags;
	int wheel;
};
mouseState mouse;

void eventKeyboard(appState& state, int stream, std::string windowTitle, cv::Mat& outputMat);
void eventMouse(int event, int x, int y, int flags, void* param);

int main(int argc, char * argv[]) try
{
	std::string appTitle = "Kagome-Kagome";
	app kagome(appTitle);
	
	kagome.setResolution(RS2_STREAM_COLOR, 1280, 720, 30);
	kagome.setResolution(RS2_STREAM_DEPTH, 1280, 720, 30);
	kagome.setVisualPreset("High Density");
	kagome.cameraInitial();
	
	cv::namedWindow(appTitle, cv::WINDOW_AUTOSIZE);
	//cv::setMouseCallback(appTitle, eventMouse, NULL);

	while (kagome.state)
	{
		kagome.cameraProcess();
		cv::imshow(appTitle, kagome.outputMat);
		eventKeyboard(kagome.state, kagome.stream, appTitle, kagome.outputMat);

		/*if (mouse.trigger)
		{
			kagome.eventMouseCV(mouse.event, mouse.x, mouse.y, mouse.flags, mouse.wheel);
			mouse.trigger = false;
		}*/
	}

	return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	system("pause");
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	system("pause");
	return EXIT_FAILURE;
}

void eventKeyboard(appState& state, int stream, std::string windowTitle, cv::Mat& outputMat)
{
	char key = cv::waitKey(10);

	if (key == 'q' || key == 'Q')
		state = APPSTATE_EXIT;
	else if (key == 's' || key == 'S')
	{
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
			state = APPSTATE_DEPTH;
			break;
		case APPSTATE_DEPTH:
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			else if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
			break;
		default:
			break;
		}
	}
	else if (key == 'a' || key == 'A')
	{
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
		case APPSTATE_DEPTH:
		case APPSTATE_RULER:
		case APPSTATE_SCANNER:
		case APPSTATE_MEASURER:
			state = APPSTATE_PHOTOGRAPHER;
			break;
		case APPSTATE_PHOTOGRAPHER:
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			else if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
			break;
		default:
			state = APPSTATE_PHOTOGRAPHER;
			break;
		}
	}
	else if (key == 'z' || key == 'Z')
	{
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
		case APPSTATE_DEPTH:
		case APPSTATE_PHOTOGRAPHER:
		case APPSTATE_SCANNER:
		case APPSTATE_MEASURER:
			state = APPSTATE_RULER;
			break;
		case APPSTATE_RULER:
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			else if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
			break;
		default:
			state = APPSTATE_RULER;
			break;
		}
	}
	else if (key == 'x' || key == 'X')
	{
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
		case APPSTATE_DEPTH:
		case APPSTATE_PHOTOGRAPHER:
		case APPSTATE_RULER:
		case APPSTATE_MEASURER:
			state = APPSTATE_SCANNER;
			break;
		case APPSTATE_SCANNER:
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			else if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
			break;
		default:
			state = APPSTATE_SCANNER;
			break;
		}
	}
	else if (key == 'w' || key == 'W')
	{
		time_t t = std::time(nullptr);
#pragma warning( disable : 4996 )
		tm tm = *std::localtime(&t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
		std::string str = windowTitle + "_" + oss.str() + ".jpg";
		cv::imwrite(str, outputMat);
		std::cout << "file saved: " << str << std::endl;
	}
	else if (key == 'c' || key == 'C')
	{
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
		case APPSTATE_DEPTH:
		case APPSTATE_PHOTOGRAPHER:
		case APPSTATE_RULER:
			state = APPSTATE_MEASURER;
			break;
		case APPSTATE_MEASURER:
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			else if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
			break;
		default:
			state = APPSTATE_MEASURER;
			break;
		}
	}


	/*static std::clock_t begin = 0;
	static std::clock_t end = 0;

	end = clock();
	elapsed = double(end - begin) * 1000 / CLOCKS_PER_SEC;
	std::ostringstream strs;
	strs << elapsed;
	std::cout << "time elapsed: " << strs.str() << std::endl;

	begin = end;*/
}

void eventMouse(int event, int x, int y, int flags, void* param)
{
	mouse.trigger = true;
	mouse.event = event;
	mouse.x = x;
	mouse.y = y;
	mouse.flags = flags;
	mouse.wheel = cv::getMouseWheelDelta(flags);
}