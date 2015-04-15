#ifndef ZENGARDEN_H
#define ZENGARDEN_H

#include <QtWidgets/QMainWindow>
#include "ui_zengarden.h"
#include "graph/operatorPanel.h"
#include "document.h"
#include <zengine.h>

class zengarden : public QMainWindow
{
	Q_OBJECT

public:
	zengarden(QWidget *parent = 0);
	~zengarden();

private:
	Ui::zengardenClass ui;
	void Log(LogMessage Message);

	OperatorPanel*				OpPanel;
	Document*					Doc;

private slots:
	void InitModules();
	void DisposeModules();
};

#endif // ZENGARDEN_H
