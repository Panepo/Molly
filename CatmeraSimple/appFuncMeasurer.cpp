#include "stdafx.h"
#include "app.h"

void app::measurerMain(cv::Mat* input, const rs2::depth_frame* depth, const rs2_intrinsics* intrin, measurerState& mstate)
{
	std::cout << "fq" << std::endl;
	switch (mstate)
	{
	case MEASURER_INIT:
		measurerMask.create(input->size(), CV_8UC1);
		measurerMask.setTo(cv::Scalar::all(cv::GC_BGD));
		mstate = MEASURER_PAINT;
	case MEASURER_PAINT:
		measurerPaint(input);
		break;
	case MEASURER_CALC:
		measurerCalc(input, depth, intrin);
		break;
	case MEASURER_RESET:
		measurerMask.release();
		mstate = MEASURER_PAINT;
	default:
		break;
	}
}

void app::measurerPaint(cv::Mat * input)
{
	std::cout << "fqfq" << std::endl;
	cv::Mat maskEdge;
	cv::Canny(measurerMask, maskEdge, 50, 150, 3);

	cv::imshow("measurerMask", measurerMask);
	cv::imshow("maskEdge", maskEdge);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(maskEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	
	if (contours.size() > 0)
	{
		cv::Mat overlay = input->clone();
		for (int i = 0; i < (int)contours.size(); i += 1)
			cv::drawContours(overlay, contours, i, scanRectColor, scanRectSize);

		cv::addWeighted(overlay, transparentP, *input, transparentO, 0, *input);
	}
}

void app::measurerCalc(cv::Mat * input, const rs2::depth_frame * depth, const rs2_intrinsics * intrin)
{
}

