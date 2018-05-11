#include "textTexture.h"
#include <QFontMetrics>
#include <QPixmap>
#include <QPainter>
#include <QGLWidget>

TextTexture::TextTexture()
{}

TextTexture::~TextTexture()
{
  DestroyTexture();
}

/// Returns the smallest value thats divisible by N and not smaller than A
template<int N>
int NextDivisibleBy(int A)
{
  return (A & ~(N - 1)) + ((A & (N - 1)) ? N : 0);
}

void TextTexture::SetText(const QString& Text, const QFont& Font)
{
  /// Calculate image size for the text
  QString text = Text == "" ? " " : Text;
  QFontMetrics fm(Font);
  QRect rect = fm.boundingRect(text);
  mTextSize = rect.size() + QSize(1, 1);
  mWidth = NextDivisibleBy<16>(mTextSize.width());
  mHeight = NextDivisibleBy<16>(mTextSize.height());

  /// Let Qt draw the text
  QPixmap pixmap(mWidth, mHeight);
  pixmap.fill(QColor(0, 0, 0, 0));
  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
  painter.setFont(Font);
  painter.setPen(Qt::white);
  painter.drawText(0, 0, mWidth, mHeight, Qt::AlignTop | Qt::AlignLeft, text);
  painter.end();
  mImage = QGLWidget::convertToGLFormat(pixmap.toImage().mirrored(false, true));
  mUptodate = false;
}


Texture* TextTexture::GetTexture() {
  if (!mUptodate) {
    /// Upload to texture
    if (mTexture == nullptr || mTexture->mWidth < mWidth || mTexture->mHeight < mHeight) {
      DestroyTexture();
      mTexture = TheResourceManager->CreateGPUTexture(
        mWidth, mHeight, TexelType::ARGB8, mImage.bits(), false, false);
    }
    else {
      OpenGL->UploadTextureSubData(
        mTexture->mHandle, 0, 0, mWidth, mHeight, TexelType::ARGB8, mImage.bits());
    }
    mUptodate = true;
  }

  return mTexture;
}

void TextTexture::DestroyTexture() {
  if (mTexture) {
    TheResourceManager->DiscardTexture(mTexture);
    /// This is awful, use shared_ptr
    mTexture = nullptr;
  }
}

