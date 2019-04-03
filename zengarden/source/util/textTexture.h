#pragma once

#include <zengine.h>
#include <QString>
#include <QFont>
#include <QSize>
#include <QImage>

class TextTexture {
public:
  TextTexture();

  void SetText(const QString& Text, const QFont& Font);
  QSize mTextSize;
  shared_ptr<Texture> GetTexture();

private:
  shared_ptr<Texture> mTexture;
  QImage mImage;

  int mWidth = 0;
  int mHeight = 0;

  bool mUptodate = false;
};