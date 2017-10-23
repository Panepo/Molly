#pragma once
#ifndef MOLICIVI_H
#define MOLICIVI_H

#include <opencv2\imgproc\types_c.h>
#include <opencv2\imgcodecs\imgcodecs.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <Robuffer.h>

using namespace std;
using namespace cv;

using namespace Windows::Graphics::Imaging;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;

namespace Molicivi
{
    public ref class MoliOpenCV sealed
    {
    public:
		MoliOpenCV();
		static WriteableBitmap^ imgProcBitmap(WriteableBitmap^ input);
	private:
		static Mat imgProc(Mat input);
		
		static Mat convertBitmapToMat(WriteableBitmap^ input);
		static byte* GetPointerToPixelBuffer(IBuffer^ pixelBuffer);
		static WriteableBitmap^ convertMatToBitmap(Mat input);
    };
}

#endif