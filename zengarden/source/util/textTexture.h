#pragma once

#include <zengine.h>
#include <QString>
#include <QFont>
#include <QSize>
#include <QImage>

class TextTexture {
public:
  TextTexture();
  ~TextTexture();

  void SetText(const QString& Text, const QFont& Font);
  QSize mTextSize;
  Texture* GetTexture();

private:
  void DestroyTexture();

  Texture* mTexture = nullptr;
  QImage mImage;

  int mWidth = 0;
  int mHeight = 0;

  bool mUptodate = false;
};