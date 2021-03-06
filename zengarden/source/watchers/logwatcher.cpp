#include "logwatcher.h"
#include <QtWidgets/QBoxLayout>
#include <QtCore/QTime>

LogWatcher::LogWatcher(QWidget* Parent)
	: QWidget(Parent)
{
	TextEdit = new QTextEdit(this);
	TextEdit->setReadOnly(true);
	QHBoxLayout *glLayout = new QHBoxLayout(this);
	glLayout->setSpacing(0);
	glLayout->setContentsMargins(0, 0, 0, 0);
	glLayout->addWidget(TextEdit);
	TheLogger->onLog += Delegate(this, &LogWatcher::Log);
}

LogWatcher::~LogWatcher() {
	TheLogger->onLog -= Delegate(this, &LogWatcher::Log);
}

void LogWatcher::Log(LogMessage Message) const {
	switch (Message.severity)
	{
	case LOG_INFO:		TextEdit->setTextColor(Qt::gray);	break;
	case LOG_WARNING:	TextEdit->setTextColor(QColor(Qt::blue).light(150));	break;
	case LOG_ERROR:		TextEdit->setTextColor(QColor(Qt::red).light(130));	break;
	}
	TextEdit->append(QString("[ ") + QTime::currentTime().toString("HH:mm:ss") + 
                   QString(" ]  ") + QString::fromUtf16(
                     reinterpret_cast<const ushort*>(Message.message)));
}

