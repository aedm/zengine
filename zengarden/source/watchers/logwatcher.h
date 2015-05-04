#pragma once

#include "watcherwidget.h"
#include <zengine.h>
#include <QtWidgets/QTextEdit>

class LogWatcher : public QWidget
{
public:
	LogWatcher(QWidget* Parent);
	~LogWatcher();

private:
	void						Log(LogMessage Message);

	QTextEdit*					TextEdit;
};