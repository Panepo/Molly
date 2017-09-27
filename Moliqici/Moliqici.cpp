// Moliqici.cpp : 定義主控台應用程式的進入點。
//
#include <iostream>

#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <OpenNI.h>

using namespace std;
using namespace openni;
using namespace cv;

void depthTrans(const Mat &src, Mat &dst) {
	dst.create(src.size(), src.type());
	//int widthLimit = src.channels() * src.cols;
	for (int iH = 0; iH<src.rows; iH++) {
		const uchar *curPtr = src.ptr<const uchar>(iH);
		uchar *dstPtr = dst.ptr<uchar>(iH);
		for (int iW = 0; iW<src.cols*2; iW++) {
			if (curPtr[iW] == 0)
				dstPtr[iW] = 0;
			else
				dstPtr[iW] = saturate_cast<uchar>(-1 * curPtr[iW] + 255);
		}
	}
}

int main(int argc, char **argv)
{
	// 1. Initial OpenNI
	if (OpenNI::initialize() != STATUS_OK)
	{	
		printf("OpenNI Initial Error: %s\n", OpenNI::getExtendedError());
		return -1;
	}

	// 2. Open Device
	Device mDevice;
	if (mDevice.open(ANY_DEVICE) != STATUS_OK)
	{
		printf("Can't Open Device: %s\n", OpenNI::getExtendedError());
		return -1;
	}

	// 3. Create depth stream
	VideoStream mDepthStream;
	if (mDevice.hasSensor(SENSOR_DEPTH))
	{
		if (mDepthStream.create(mDevice, SENSOR_DEPTH) == STATUS_OK)
		{
			// 3a. set video mode
			VideoMode mMode;
			mMode.setResolution(640, 480);
			mMode.setFps(30);
			mMode.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);

			if (mDepthStream.setVideoMode(mMode) != STATUS_OK)
			{
				printf("Can't apply VideoMode: %s\n", OpenNI::getExtendedError());
			}
		}
		else
		{
			printf("Can't create depth stream on device: %s\n", OpenNI::getExtendedError());
			return -1;
		}
	}
	else
	{
		printf("ERROR: This device does not have depth sensor\n");
		return -1;
	}

	// 4. Create color stream
	VideoStream mColorStream;
	if (mDevice.hasSensor(SENSOR_COLOR))
	{
		if (mColorStream.create(mDevice, SENSOR_COLOR) == STATUS_OK)
		{
			// 4a. set video mode
			VideoMode mMode;
			mMode.setResolution(640, 480);
			mMode.setFps(30);
			mMode.setPixelFormat(PIXEL_FORMAT_RGB888);

			if (mColorStream.setVideoMode(mMode) != STATUS_OK)
			{
				printf("Can't apply VideoMode: %s\n", OpenNI::getExtendedError());
			}

			// 4b. image registration
			if (mDevice.isImageRegistrationModeSupported(
				IMAGE_REGISTRATION_DEPTH_TO_COLOR))
			{
				mDevice.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
			}
		}
		else
		{
			printf("Can't create color stream on device: %s\n", OpenNI::getExtendedError());
			return -1;
		}
	}

	// 5. create OpenCV Window
	cv::namedWindow("Moliqici: Depth Image", CV_WINDOW_AUTOSIZE);
	cv::namedWindow("Moliqici: Color Image", CV_WINDOW_AUTOSIZE);

	// 6. start
	VideoFrameRef  mColorFrame;
	VideoFrameRef  mDepthFrame;
	mDepthStream.start();
	mColorStream.start();
	int iMaxDepth = mDepthStream.getMaxPixelValue();
	int iMinDepth = mDepthStream.getMinPixelValue();
	while (true)
	{
		// 7. check is color stream is available
		if (mColorStream.isValid())
		{
			// 7a. get color frame
			if (mColorStream.readFrame(&mColorFrame) == STATUS_OK)
			{
				// 7b. convert data to OpenCV format
				const cv::Mat mImageRGB(
					mColorFrame.getHeight(), mColorFrame.getWidth(),
					CV_8UC3, (void*)mColorFrame.getData());
				// 7c. convert form RGB to BGR
				cv::Mat cImageBGR;
				cv::cvtColor(mImageRGB, cImageBGR, CV_RGB2BGR);
				// 7d. show image
				cv::imshow("Moliqici: Color Image", cImageBGR);
			}
		}

		// 8a. get depth frame
		if (mDepthStream.readFrame(&mDepthFrame) == STATUS_OK)
		{
			// 8b. convert data to OpenCV format
			const cv::Mat mImageDepth(
				mDepthFrame.getHeight(), mDepthFrame.getWidth(),
				CV_16UC1, (void*)mDepthFrame.getData());
			// 8c. re-map depth data [0,Max] to [0,255]
			cv::Mat mScaledDepth;
			mImageDepth.convertTo(mImageDepth, CV_16U, 65535.0 / iMaxDepth);
			depthTrans(mImageDepth, mScaledDepth);

			// 8d. show image
			cv::imshow("Moliqici: Depth Image", mScaledDepth);
		}

		// 6a. check keyboard
		if (cv::waitKey(1) == 'q')
			break;
	}

	// 9. stop
	mDepthStream.destroy();
	mColorStream.destroy();
	mDevice.close();
	OpenNI::shutdown();

	return 0;
}
