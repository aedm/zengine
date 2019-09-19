#include "textTexture.h"
#include <QFontMetrics>
#include <QPixmap>
#include <QPainter>
#include <QGLWidget>

TextTexture::TextTexture() = default;

/// Returns the smallest value divisible by N and not smaller than A
template<int N>
int NextDivisibleBy(int A)
{
  return (A & ~(N - 1)) + ((A & (N - 1)) ? N : 0);
}

void TextTexture::SetText(const QString& Text, const QFont& Font)
{
  /// Calculate image size for the text
  const QString text = Text == "" ? " " : Text;
  const QFontMetrics fm(Font);
  const QRect rect = fm.boundingRect(text);
  mTextSize = rect.size() + QSize(1, 1);
  mWidth = NextDivisibleBy<16>(mTextSize.width());
  mHeight = NextDivisibleBy<16>(mTextSize.height());

  /// Let Qt draw the text
  QPixmap pixMap(mWidth, mHeight);
  pixMap.fill(QColor(0, 0, 0, 0));
  QPainter painter;
  painter.begin(&pixMap);
  painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
  painter.setFont(Font);
  painter.setPen(Qt::white);
  painter.drawText(0, 0, mWidth, mHeight, Qt::AlignTop | Qt::AlignLeft, text);
  painter.end();
  mImage = QGLWidget::convertToGLFormat(pixMap.toImage().mirrored(false, true));
  mUptodate = false;
}


shared_ptr<Texture> TextTexture::GetTexture() {
  if (!mUptodate) {
    if (mTexture == nullptr || mTexture->mWidth != mWidth || mTexture->mHeight != mHeight) {
      mTexture = OpenGL->MakeTexture(mWidth, mHeight, TexelType::ARGB8, nullptr, true,
        false, false, false);
    }
    OpenGL->UploadTextureGPUData(mTexture, mImage.bits());
    mUptodate = true;
  }

  return mTexture;
}

