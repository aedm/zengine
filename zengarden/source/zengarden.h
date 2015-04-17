#ifndef ZENGARDEN_H
#define ZENGARDEN_H

#include <QtWidgets/QMainWindow>
#include <QtOpenGL/QGLWidget>
#include "ui_zengarden.h"
#include "graph/grapheditor.h"
#include "document.h"
#include <zengine.h>

class ZenGarden : public QMainWindow
{
	Q_OBJECT

public:
	ZenGarden(QWidget *parent = 0);
	~ZenGarden();

private:

	/// Open viewers
	GraphEditor*				OpenGraphViewer(bool LeftPanel, NodeGraph* Graph);

	/// The GL widget used for initializing OpenGL and sharing context
	QGLWidget*					CommonGLWidget;

	/// Handles Zengine log messages
	void						Log(LogMessage Message);

	//GraphEditor*				TheGraphEditor;
	Document*					Doc;

	/// App UI
	Ui::zengardenClass ui;

private slots:
	void InitModules();
	void DisposeModules();
};

#endif // ZENGARDEN_H
