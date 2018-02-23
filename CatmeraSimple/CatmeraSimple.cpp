// CatmeraSimple.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <iostream>

#include "app.h"


int main(int argc, char * argv[]) try
{
	std::string appTitle = "Molly Catmera";
	app catmera(appTitle);
	
	catmera.setResolution(RS2_STREAM_COLOR, 1280, 720, 30);
	catmera.setResolution(RS2_STREAM_DEPTH, 1280, 720, 30);
	catmera.setVisualPreset("High Density");
	catmera.cameraInitial();
	
	cv::namedWindow(appTitle, cv::WINDOW_AUTOSIZE);

	while (catmera.state)
	{
		catmera.cameraProcess();
		cv::imshow(appTitle, catmera.outputMat);
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

