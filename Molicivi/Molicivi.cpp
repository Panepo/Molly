#include "pch.h"
#include "Molicivi.h"

using namespace Molicivi;
using namespace Platform;

MoliOpenCV::MoliOpenCV()
{
}

WriteableBitmap^ MoliOpenCV::imgProcBitmap(WriteableBitmap^ input)
{
	Mat matInput = convertBitmapToMat(input);
	Mat matFilter = imgProc(matInput);
	return convertMatToBitmap(matFilter);
}

Mat MoliOpenCV::imgProc(Mat input)
{
	Mat matGray = Mat(input.rows, input.cols, CV_8UC4);
	Mat matGrayC = Mat(input.rows, input.cols, CV_8UC1);
	Mat matDst;

	cvtColor(input, matGray, CV_BGR2GRAY);

	double minGray, maxGray;
	minMaxLoc(matGray, &minGray, &maxGray);

	matGray.convertTo(matDst, CV_8U, 255 / (maxGray - minGray), -1 * minGray);

	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(4);
	clahe->apply(matGray, matDst);

	cvtColor(matDst, matDst, CV_GRAY2BGRA);

	return matDst;
}

// =================================================================================
// format conversion
// =================================================================================

Mat MoliOpenCV::convertBitmapToMat(WriteableBitmap^ input)
{
	auto image = GetPointerToPixelBuffer(input->PixelBuffer);

	return Mat(input->PixelHeight, input->PixelWidth, CV_8UC4, image);
}

byte* MoliOpenCV::GetPointerToPixelBuffer(IBuffer^ pixelBuffer)
{
	ComPtr<IBufferByteAccess> bufferByteAccess;

	reinterpret_cast<IInspectable*>(pixelBuffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));

	byte* pixels = nullptr;
	bufferByteAccess->Buffer(&pixels);

	return pixels;
}

WriteableBitmap^ MoliOpenCV::convertMatToBitmap(Mat input)
{
	WriteableBitmap^ bitmap = ref new WriteableBitmap(input.cols, input.rows);

	IBuffer^ buffer = bitmap->PixelBuffer;
	unsigned char* dstPixels = nullptr;

	ComPtr<IBufferByteAccess> pBufferByteAccess;
	ComPtr<IInspectable> pBuffer((IInspectable*)buffer);
	pBuffer.As(&pBufferByteAccess);

	HRESULT get_bytes = pBufferByteAccess->Buffer(&dstPixels);
	if (get_bytes == S_OK) {
		memcpy(dstPixels, input.data, input.step.buf[1] * input.cols*input.rows);
		return bitmap;
	}
}
