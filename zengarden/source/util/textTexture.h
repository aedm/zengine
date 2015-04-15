#pragma once

#include <zengine.h>
#include <QString>
#include <QFont>
#include <QSize>

class TextTexture
{
public:
	TextTexture();
	~TextTexture();

	void				SetText(const QString& Text, const QFont& Font);
	Texture*			TheTexture;
	QSize				TextSize;

private:
	void				DestroyTexture();
};