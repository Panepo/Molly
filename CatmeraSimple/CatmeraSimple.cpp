// CatmeraSimple.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <iostream>

#include "app.h"


int main(int argc, char * argv[]) try
{
	app catmera;
	
	while (catmera.state)
	{
		if (catmera.state == APPSTATE_COLOR)
			catmera.stateColor();
		else if (catmera.state == APPSTATE_INFRARED)
			catmera.stateInfrared();
		else if (catmera.state == APPSTATE_DEPTH)
			catmera.stateDepth();
		
		catmera.stateSwitch();
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

