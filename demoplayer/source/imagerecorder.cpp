#include <Windows.h>
#include <gdiplus.h>
#include <gdiplusflat.h>
#pragma comment (lib,"Gdiplus.lib")
#include "imagerecorder.h"

using namespace Gdiplus;
using namespace Gdiplus::DllExports;

int GetEncoderClsId(const WCHAR* format, CLSID* pClsId)
{
  UINT num = 0;          // number of image encoders
  UINT size = 0;         // size of the image encoder array in bytes

  GetImageEncodersSize(&num, &size);
  if (size == 0)
    return -1;  // Failure

  ImageCodecInfo* pImageCodecInfo = static_cast<ImageCodecInfo*>(malloc(size));
  if (pImageCodecInfo == nullptr)
    return -1;  // Failure

  GetImageEncoders(num, size, pImageCodecInfo);

  for (UINT j = 0; j < num; ++j) {
    if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
      *pClsId = pImageCodecInfo[j].Clsid;
      free(pImageCodecInfo);
      return j;  // Success
    }
  }

  free(pImageCodecInfo);
  return -1;  // Failure
}

ImageRecorder::ImageRecorder()
{
  GdiplusStartupInput gdiPlusStartupInput;
  ULONG_PTR gdiPlusToken;
  GdiplusStartup(&gdiPlusToken, &gdiPlusStartupInput, nullptr);

  GetEncoderClsId(L"image/jpeg", &mImageClsId);
}

ImageRecorder::~ImageRecorder()
= default;

void ImageRecorder::RecordImage(unsigned char* pixels, int width, int height, 
  int frameNumber) const
{
  GpBitmap* pBitmap;
  GdipCreateBitmapFromScan0(width, height, width * 4, PixelFormat32bppARGB, 
    pixels, &pBitmap);

  ULONG uQuality = 99;
  EncoderParameters encoderParams{};
  encoderParams.Count = 1;
  encoderParams.Parameter[0].NumberOfValues = 1;
  encoderParams.Parameter[0].Guid = EncoderQuality;
  encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
  encoderParams.Parameter[0].Value = &uQuality;

  wchar_t filename[100];
  wsprintf(filename, L"videodump-%08d.jpeg", frameNumber);

  GdipSaveImageToFile(pBitmap, filename, &mImageClsId, &encoderParams);
  GdipDisposeImage(pBitmap);
}
