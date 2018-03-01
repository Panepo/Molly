#include "stdafx.h"
#include "app.h"

app::app(std::string title)
{
	windowTitle = title;
}

void app::cameraInitial()
{
	stream = detectDevice();
	if (stream & EnableColor)
	{
		config.enable_stream(RS2_STREAM_COLOR, ColorWidth, ColorHeight, RS2_FORMAT_BGR8, ColorFPS);
		state = APPSTATE_COLOR;
		//state = APPSTATE_MEASURE;
	}
	else if (stream & EnableInfrared)
	{
		config.enable_stream(RS2_STREAM_INFRARED, DepthWidth, DepthHeight, RS2_FORMAT_Y8, DepthFPS);
		state = APPSTATE_INFRARED;
	}

	config.enable_stream(RS2_STREAM_DEPTH, DepthWidth, DepthHeight, RS2_FORMAT_Z16, DepthFPS);

	rs2::pipeline_profile cfg = pipeline.start(config);

	auto sensor = cfg.get_device().first<rs2::depth_sensor>();

	auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
	for (auto i = range.min; i < range.max; i += range.step)
		if (std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) == visualPreset)
			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);

	auto profile = cfg.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();
	
	if (stream & EnableInfrared)
		profile = cfg.get_stream(RS2_STREAM_INFRARED).as<rs2::video_stream_profile>();
	
	intrinsics = profile.get_intrinsics();

	filterSpat.set_option(RS2_OPTION_HOLES_FILL, 5);

	pixelA[0] = (float)ColorWidth * 3 / 5;
	pixelA[1] = (float)ColorHeight / 2;
	pixelB[0] = (float)ColorWidth * 2 / 5;
	pixelB[1] = (float)ColorHeight / 2;
}

void app::cameraProcess()
{
	std::clock_t begin = clock();
	cv::setMouseCallback(windowTitle, eventMouseS, this);
	
	switch (state)
	{
	case APPSTATE_COLOR:
		stateColor();
		break;
	case APPSTATE_INFRARED:
		stateInfrared();
		break;
	case APPSTATE_DEPTH:
		stateDepth();
		break;
	case APPSTATE_MEASURE:
		stateMeasure();
		break;
	default:
		state = APPSTATE_EXIT;
		break;
	}

	eventKeyboard();
	clock_t end = clock();
	elapsed = double(end - begin) * 1000 / CLOCKS_PER_SEC;
}

// =================================================================================
// Application states
// =================================================================================

void app::stateColor()
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);
	
	streamPostProcess(&colorMat, &depth);
}

void app::stateInfrared()
{
	rs2::align alignTo(RS2_STREAM_INFRARED);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);
	
	streamPostProcess(&infraredMat, &depth);
}

void app::stateDepth()
{
	rs2::colorizer colorize;
	//colorize.set_option(RS2_OPTION_COLOR_SCHEME, 2);
	
	rs2::align alignTo(RS2_STREAM_COLOR);
	if (stream & EnableInfrared)
		rs2::align alignTo(RS2_STREAM_INFRARED);

	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);
	rs2::frame depthColor = colorize(depth);
	cv::Mat depthMat = funcFormat::frame2Mat(depthColor);

	streamPostProcess(&depthMat, &depth);
}

void app::stateMeasure()
{
	rs2::frameset data = pipeline.wait_for_frames();
	
	if (stream & EnableColor)
	{
		rs2::align alignTo(RS2_STREAM_COLOR);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat colorMat = funcFormat::frame2Mat(alignedFrame.get_color_frame());
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);
		measurePostProcess(&colorMat, &depth);
	}
	else
	{
		rs2::align alignTo(RS2_STREAM_INFRARED);
		rs2::frameset alignedFrame = alignTo.process(data);

		cv::Mat infraredMat = funcFormat::frame2Mat(alignedFrame.first(RS2_STREAM_INFRARED));
		rs2::depth_frame depth = alignedFrame.get_depth_frame();
		depth = filterSpat.process(depth);
		depth = filterTemp.process(depth);
		measurePostProcess(&infraredMat, &depth);
	}
}

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
		if (state == APPSTATE_COLOR)
			state = APPSTATE_DEPTH;
		else if (state == APPSTATE_INFRARED)
			state = APPSTATE_DEPTH;
		else if (state == APPSTATE_DEPTH)
		{
			if (stream & EnableColor)
				state = APPSTATE_COLOR;
			if (stream & EnableInfrared)
				state = APPSTATE_INFRARED;
		}
	}
	else if (key == 'a' || key == 'A')
	{
		if (state == APPSTATE_MEASURE)
			state = APPSTATE_COLOR;
		else
			state = APPSTATE_MEASURE;
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
			if (y >= 0 && y<= ColorHeight)
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

// =================================================================================
// Application settings
// =================================================================================

void app::setResolution(int stream, int width, int height, int fps)
{
	switch (stream)
	{
	case RS2_STREAM_COLOR:
		ColorWidth = width;
		ColorHeight = height;
		ColorFPS = fps;
		break;
	case RS2_STREAM_INFRARED:
	case RS2_STREAM_DEPTH:
		DepthWidth = width;
		DepthHeight = height;
		DepthFPS = fps;
		break;
	default:
		break;
	}
}


void app::setVisualPreset(std::string preset)
{
	visualPreset = preset;
}

// =================================================================================
// Application major private functions
// =================================================================================

void app::streamPostProcess(cv::Mat* input, rs2::depth_frame* depth)
{
	outputMat = streamZoomer(input);
	
	streamPointer(&outputMat, depth, &intrinsics);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms (" + std::to_string((int)point[0]) + " "
		+ std::to_string((int)point[1]) + " " + std::to_string((int)point[2]) + ")";
	streamInfoer(&outputMat, str);
}

void app::measurePostProcess(cv::Mat * input, rs2::depth_frame * depth)
{
	outputMat = streamZoomer(input);

	measurePointer(&outputMat, depth, &intrinsics);
	measureDrawer(&outputMat, depth, &intrinsics);

	std::ostringstream strs;
	strs << elapsed;
	std::string str = strs.str() + " ms " + distText;
	streamInfoer(&outputMat, str);
}

// =================================================================================
// Application minor private functions
// =================================================================================

void app::streamPointer(cv::Mat* input, rs2::depth_frame* depth, rs2_intrinsics* intrin)
{
	float pos[2];
	
	if (scaleZoom == 1)
	{
		pos[0] = pixel[0];
		pos[1] = pixel[1];
	}
	else
	{
		pos[0] = pixel[0] * scaleZoom + roiZoom[0];
		pos[1] = pixel[1] * scaleZoom + roiZoom[1];
	}
	
	float depthPixel = depth->get_distance((int)pos[0], (int)pos[1]);
	rs2_deproject_pixel_to_point(point, intrin, pos, depthPixel * 1000);

	cv::circle(*input, cv::Point((int)pixel[0], (int)pixel[1]), pointerSize, pointerColor, -1);
	std::string text = std::to_string((int)point[0]) + " " + std::to_string((int)point[1])
		+ " " + std::to_string((int)point[2]);

	cv::putText(*input, text, cv::Point((int)pixel[0] - 100, (int)pixel[1] + 40), pointerFontA, 
		1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(*input, text, cv::Point((int)pixel[0] - 100, (int)pixel[1] + 40), pointerFontB, 
		1, pointerColorFB, 1, cv::LINE_AA);
}

void app::streamInfoer(cv::Mat* input, std::string text)
{
	cv::Size size = input->size();
	cv::copyMakeBorder(*input, *input, 0, 40, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	cv::putText(*input, text, cv::Point(10, size.height + 30), inforerFont, 1, inforerColor, 1, cv::LINE_AA);
}

cv::Mat app::streamZoomer(cv::Mat* input)
{
	if (scaleZoom == 1)
	{
		cv::Mat output = input->clone();
		return output;
	}
	else
	{
		// generate zoom-in image
		pixelZoom[0] = (int)(pixelZoom[0] * scaleZoom) + roiZoom[0];
		pixelZoom[1] = (int)(pixelZoom[1] * scaleZoom) + roiZoom[1];

		cv::Size size = input->size();
		int scaledWidth = (int)(size.width * scaleZoom);
		int scaledHeight = (int)(size.height * scaleZoom);
		roiZoom[0] = pixelZoom[0] - (scaledWidth / 2);
		roiZoom[1] = pixelZoom[1] - (scaledHeight / 2);
		
		if ((pixelZoom[0] + (scaledWidth / 2)) > size.width)
			roiZoom[0] = size.width - scaledWidth;

		if ((pixelZoom[1] + (scaledHeight / 2)) > size.height)
			roiZoom[1] = size.height - scaledHeight;

		if ((pixelZoom[0] - (scaledWidth / 2)) < 0)
			roiZoom[0] = 0;

		if ((pixelZoom[1] - (scaledHeight / 2)) < 0)
			roiZoom[1] = 0;

		cv::Rect roi = cv::Rect(roiZoom[0], roiZoom[1], scaledWidth, scaledHeight);
		cv::Mat output = (*input)(roi);
		cv::resize(output, output, size, 0, 0, CV_INTER_AREA);

		// generate mini-map
		cv::Mat outMap = input->clone();
		cv::rectangle(outMap, roi, zoomerLineColor, zoomerLineSize);
		cv::Size sizeMap = cv::Size((int)(size.width / 8), (int)(size.height / 8));
		cv::resize(outMap, outMap, sizeMap, 0, 0, CV_INTER_AREA);
		cv::copyMakeBorder(outMap, outMap, zoomerMapSize, zoomerMapSize, zoomerMapSize, 
			zoomerMapSize, cv::BORDER_CONSTANT, zoomerMapColor);
		outMap.copyTo(output(cv::Rect(10, size.height - sizeMap.height - 10, 
			outMap.cols, outMap.rows)));

		return output;
	}
}

void app::measurePointer(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin)
{
	float posA[2], posB[2];

	if (scaleZoom == 1)
	{
		posA[0] = pixelA[0];
		posA[1] = pixelA[1];
		posB[0] = pixelB[0];
		posB[1] = pixelB[1];
	}
	else
	{
		posA[0] = pixelA[0] * scaleZoom + roiZoom[0];
		posA[1] = pixelA[1] * scaleZoom + roiZoom[1];
		posB[0] = pixelB[0] * scaleZoom + roiZoom[0];
		posB[1] = pixelB[1] * scaleZoom + roiZoom[1];
	}

	cv::circle(*input, cv::Point((int)pixel[0], (int)pixel[1]), pointerSize, pointerColor, -1);
	cv::circle(*input, cv::Point((int)pixelA[0], (int)pixelA[1]), measureSize, measureColor, 2);
	cv::circle(*input, cv::Point((int)pixelB[0], (int)pixelB[1]), measureSize, measureColor, 2);
	cv::line(*input, cv::Point((int)pixelA[0], (int)pixelA[1]), 
		cv::Point((int)pixelB[0], (int)pixelB[1]), measureColor, 2);

	float dist = measureDist(intrin, depth, posA, posB);

	std::ostringstream strs;
	strs << dist;
	distText = strs.str() + "cm";

	int textCoordX = (int)((pixelA[0] + pixelB[0]) / 2);
	int textCoordY = (int)((pixelA[1] + pixelB[1]) / 2);
	
	cv::putText(*input, distText, cv::Point(textCoordX, textCoordY),
		pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(*input, distText, cv::Point(textCoordX, textCoordY),
		pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
}

float app::measureDist(const rs2_intrinsics* intrin, const rs2::depth_frame* depth, float pixelA[3], float pixelB[3])
{
	float pointA[3] = { 0, 0, 0 };
	float pointB[3] = { 0, 0, 0 };

	float distA = depth->get_distance((int)pixelA[0], (int)pixelA[1]);
	float distB = depth->get_distance((int)pixelB[0], (int)pixelB[1]);
	
	rs2_deproject_pixel_to_point(pointA, intrin, pixelA, distA);
	rs2_deproject_pixel_to_point(pointB, intrin, pixelB, distB);

	return floor(sqrt(pow(pointA[0] - pointB[0], 2) +
		pow(pointA[1] - pointB[1], 2) +
		pow(pointA[2] - pointB[2], 2)) * 10000) / 100;
}

void app::measureDrawer(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
	float posA[2], posB[2];

	if (scaleZoom == 1)
	{
		posA[0] = pixelA[0];
		posA[1] = pixelA[1];
		posB[0] = pixelB[0];
		posB[1] = pixelB[1];
	}
	else
	{
		posA[0] = pixelA[0] * scaleZoom + roiZoom[0];
		posA[1] = pixelA[1] * scaleZoom + roiZoom[1];
		posB[0] = pixelB[0] * scaleZoom + roiZoom[0];
		posB[1] = pixelB[1] * scaleZoom + roiZoom[1];
	}
	
	float xdiff = abs(posA[0] - posB[0]);
	float ydiff = abs(posA[1] - posB[1]);
	int posX = 0, posY = 0;
	float dist = 0, parm = 1;
	float directX = 1, directY = 1;
	std::vector<float> output;

	// get depth data of the route
	if (posA[0] - posB[0] < 0)
		directX = -1;
	
	if (posA[1] - posB[1] < 0)
		directY = -1;

	if (xdiff < ydiff)
	{
		for (int i : boost::irange<int>(0, (int)ydiff))
		{
			posX = (int)floor(posB[0] + directX * (float)i * xdiff / ydiff);
			posY = (int)floor(posB[1] + directY * (float)i);
			dist = depth->get_distance(posX, posY);
			output.push_back(dist);
		}
	}
	else
	{
		for (int i : boost::irange<int>(0, (int)xdiff))
		{
			posX = (int)floor(posB[0] + directX * (float)i);
			posY = (int)floor(posB[1] + directY * (float)i * ydiff / xdiff);
			dist = depth->get_distance(posX, posY);
			output.push_back(dist);
		}
	}

	if (output.size() > 0)
	{
		// get min and max value to calculate drawing parameter
		auto outMinMax = std::minmax_element(output.begin(), output.end());

		if ((float)(*outMinMax.second - *outMinMax.first) < 0.2)
			parm = sectionHeight * 5;
		else
			parm = sectionHeight / (*outMinMax.second - *outMinMax.first);
		
		// draw sectional drawing
		cv::Mat minimap = cv::Mat(sectionHeight, (int)output.size(), CV_8UC3, sectionColor);
		for (int i : boost::irange<int>(0, (int)output.size()))
		{
			cv::line(minimap, cv::Point(i, 0),
				cv::Point(i, (int)((output[i] - *outMinMax.first) * parm)), sectionBGColor, 1);
		}

		// flip image if inverse
		if (posA[0] < posB[0])
		{
			cv::Mat minimapFlip;
			cv::flip(minimap, minimapFlip, 1);
			minimap = minimapFlip.clone();
		}

		// generate preserve border
		cv::copyMakeBorder(minimap, minimap, sectionPreSize, 0, 0, 0, cv::BORDER_CONSTANT, sectionBGColor);
		cv::copyMakeBorder(minimap, minimap, 0, sectionPreSize, 0, 0, cv::BORDER_CONSTANT, sectionColor);
		
		// generate minimap
		cv::Size size = input->size();
		cv::Size sizeMap = cv::Size((int)(size.width / 8), (int)(size.height / 8));
		cv::resize(minimap, minimap, sizeMap, 0, 0, CV_INTER_LINEAR);
		cv::copyMakeBorder(minimap, minimap, sectionMapSize, sectionMapSize, sectionMapSize,
			sectionMapSize, cv::BORDER_CONSTANT, sectionMapColor);

		cv::Mat outMat = input->clone();
		minimap.copyTo(outMat(cv::Rect(size.width - sizeMap.width - 10, size.height - sizeMap.height - 10,
		minimap.cols, minimap.rows)));
		*input = outMat.clone();
	}
}
