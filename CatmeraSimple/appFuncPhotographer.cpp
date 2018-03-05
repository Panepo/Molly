#include "stdafx.h"
#include "app.h"

auto gen_element = [](int erosion_size)
{
	return getStructuringElement(cv::MORPH_RECT,
		cv::Size(erosion_size + 1, erosion_size + 1),
		cv::Point(erosion_size, erosion_size));
};

const int erosion_size = 3;
auto erode_less = gen_element(erosion_size);
auto erode_more = gen_element(erosion_size * 2);

auto create_mask_from_depth = [&](cv::Mat& depth, int thresh, cv::ThresholdTypes type)
{
	cv::threshold(depth, depth, thresh, 255, type);
	cv::dilate(depth, depth, erode_less);
	cv::erode(depth, depth, erode_more);
};


// =================================================================================
// Application minor private functions for aligner
// =================================================================================

void app::photographerRenderer(cv::Mat * input, cv::Mat * depth)
{
	cv::Mat near = depth->clone();
	cv::Mat far = depth->clone();

	#pragma omp parallel sections
	{
		#pragma omp section
		{		
			cv::cvtColor(near, near, CV_BGR2GRAY);
			create_mask_from_depth(near, 180, cv::THRESH_BINARY);
		}
		
		#pragma omp section
		{	
			cv::cvtColor(far, far, CV_BGR2GRAY);
			far.setTo(255, far == 0);
			create_mask_from_depth(far, 100, cv::THRESH_BINARY_INV);
		}
	}

	cv::Mat mask;
	mask.create(near.size(), CV_8UC1);
	mask.setTo(cv::Scalar::all(cv::GC_BGD)); 
	mask.setTo(cv::GC_PR_BGD, far == 0);
	mask.setTo(cv::GC_FGD, near == 255);

	cv::Mat bgModel, fgModel;
	cv::grabCut(*input, mask, cv::Rect(), bgModel, fgModel, 1, cv::GC_INIT_WITH_MASK);

	cv::Mat3b foreground = cv::Mat3b::zeros(input->rows, input->cols);
	input->copyTo(foreground, (mask == cv::GC_FGD) | (mask == cv::GC_PR_FGD));
	*input = foreground.clone();
}


