#include "textTexture.h"
#include <QFontMetrics>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QGLWidget>

TextTexture::TextTexture()
	: TheTexture(NULL)
{}

TextTexture::~TextTexture()
{
	DestroyTexture();
}

/// Returns the smallest value thats divisible by N and not smaller than A
template<int N>
int NextDivisibleBy(int A)
{
	return (A & ~(N-1)) + ((A & (N-1)) ? N : 0);
}

void TextTexture::SetText( const QString& Text, const QFont& Font )
{
	/// Calculate image size for the text
	QFontMetrics fm(Font);
	QRect rect = fm.boundingRect(Text);
	TextSize = rect.size() + QSize(1,1);
	int tw = NextDivisibleBy<16>(TextSize.width());
	int th = NextDivisibleBy<16>(TextSize.height());

	/// Let Qt draw the text
	QPixmap pixmap(tw, th);
	pixmap.fill(QColor(0, 0, 0, 0));
	QPainter painter;
	painter.begin(&pixmap);
	painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
	painter.setFont(Font);
	painter.setPen(Qt::white);
	painter.drawText(0, 0, tw, th, Qt::AlignTop | Qt::AlignLeft, Text);
	painter.end();
	QImage image = QGLWidget::convertToGLFormat(pixmap.toImage().mirrored(false, true));

	/// Upload to texture
	if (TheTexture == NULL || TheTexture->mWidth < tw || TheTexture->mHeight < th)
	{
		DestroyTexture();
		TheTexture = TheResourceManager->CreateTexture(tw, th, TEXELTYPE_RGBA_UINT8, image.bits());
	} else {
		TheDrawingAPI->UploadTextureSubData(TheTexture->mHandle, 0, 0, tw, th, TEXELTYPE_RGBA_UINT8, 
			image.bits());
	}
}

void TextTexture::DestroyTexture()
{
	if (TheTexture) 
	{
		TheResourceManager->DiscardTexture(TheTexture);
		TheTexture = NULL;
	}
}

