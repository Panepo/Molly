#include "stdafx.h"
#include "app.h"

// =================================================================================
// Application events
// =================================================================================

void app::eventKeyboard()
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
			state = APPSTATE_MEASURE;
			break;
		case APPSTATE_MEASURE:
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			else if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
			break;
		default:
			break;
		}
	}
}

void app::eventMouseS(int event, int x, int y, int flags, void* userdata)
{
	app* temp = reinterpret_cast<app*>(userdata);
	temp->eventMouse(event, x, y, flags);
}

void app::eventMouse(int event, int x, int y, int flags)
{
	float value;

	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		switch (state)
		{
		case APPSTATE_COLOR:
			if (x >= 0 && x <= ColorWidth)
				pixel[0] = (float)x;
			if (y >= 0 && y <= ColorHeight)
				pixel[1] = (float)y;
		case APPSTATE_INFRARED:
			if (x >= 0 && x <= DepthWidth)
				pixel[0] = (float)x;
			if (y >= 0 && y <= DepthHeight)
				pixel[1] = (float)y;
		case APPSTATE_DEPTH:
		case APPSTATE_MEASURE:
			if (stream & EnableColor)
			{
				if (x >= 0 && x <= ColorWidth)
					pixel[0] = (float)x;
				if (y >= 0 && y <= ColorHeight)
					pixel[1] = (float)y;
			}
			else if (stream & EnableInfrared)
			{
				if (x >= 0 && x <= DepthWidth)
					pixel[0] = (float)x;
				if (y >= 0 && y <= DepthHeight)
					pixel[1] = (float)y;
			}
			break;
		default:
			break;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		switch (state)
		{
		case APPSTATE_MEASURE:
			if (stream & EnableColor)
			{
				if (x >= 0 && x <= ColorWidth)
					pixelA[0] = (float)x;
				else
					pixelA[0] = (float)ColorWidth - 1;
				if (y >= 0 && y <= ColorHeight)
					pixelA[1] = (float)y;
				else
					pixelA[1] = (float)ColorHeight - 1;
			}
			else if (stream & EnableInfrared)
			{
				if (x >= 0 && x <= DepthWidth)
					pixelA[0] = (float)x;
				else
					pixelA[0] = (float)DepthWidth - 1;
				if (y >= 0 && y <= DepthHeight)
					pixelA[1] = (float)y;
				else
					pixelA[1] = (float)DepthHeight - 1;
			}
			break;
		}
		break;
	case CV_EVENT_RBUTTONDOWN:
		switch (state)
		{
		case APPSTATE_MEASURE:
			if (stream & EnableColor)
			{
				if (x >= 0 && x <= ColorWidth)
					pixelB[0] = (float)x;
				else
					pixelB[0] = (float)ColorWidth - 1;
				if (y >= 0 && y <= ColorHeight)
					pixelB[1] = (float)y;
				else
					pixelB[1] = (float)ColorHeight - 1;
			}
			else if (stream & EnableInfrared)
			{
				if (x >= 0 && x <= DepthWidth)
					pixelB[0] = (float)x;
				else
					pixelB[0] = (float)DepthWidth - 1;
				if (y >= 0 && y <= DepthHeight)
					pixelB[1] = (float)y;
				else
					pixelB[1] = (float)DepthHeight - 1;
			}
			break;
		}
		break;
	case CV_EVENT_MOUSEWHEEL:
		switch (state)
		{
		case APPSTATE_COLOR:
		case APPSTATE_INFRARED:
		case APPSTATE_DEPTH:
		case APPSTATE_MEASURE:
			pixelZoom[0] = x;
			pixelZoom[1] = y;

			value = (float)cv::getMouseWheelDelta(flags);
			//std::cout << value << std::endl;
			if (value > 0 && scaleZoom < zoomerScaleMax)
				scaleZoom += (float) 0.1;
			else if (value < 0 && scaleZoom > zoomerScaleMin)
				scaleZoom -= (float) 0.1;
			break;
		default:
			break;
		}

	default:
		break;
	}
}