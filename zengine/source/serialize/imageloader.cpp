#include <include/serialize/imageloader.h>
#include <include/render/drawingapi.h>

#include <Windows.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;
using namespace Gdiplus::DllExports;

void Zengine::InitGDIPlus() {
  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

shared_ptr<Texture> Zengine::LoadTextureFromFile(const wstring& fileName) {
  Gdiplus::Bitmap bitmap(fileName.c_str());
  UINT width = bitmap.GetWidth();
  UINT height = bitmap.GetHeight();

  /// Lock entire region to get the pixels
  Gdiplus::Rect rect(0, 0, width, height);
  auto *bitmapData = new Gdiplus::BitmapData;
  bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData);
  char* pixels = static_cast<char*>(bitmapData->Scan0);

  auto texture = 
    OpenGL->MakeTexture(width, height, TexelType::ARGB8, pixels, true, false, true, true);
  
  bitmap.UnlockBits(bitmapData);
  return texture;
}
