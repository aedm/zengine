#pragma once

class ImageRecorder {
public:
  ImageRecorder();
  ~ImageRecorder();
  
  void RecordImage(unsigned char* pixels, int width, int height, int frameNumber) const;

private:
  CLSID mImageClsId{};
};
