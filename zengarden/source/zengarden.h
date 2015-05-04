#ifndef ZENGARDEN_H
#define ZENGARDEN_H

#include <QtWidgets/QMainWindow>
#include <QtOpenGL/QGLWidget>
#include "ui_zengarden.h"
#include "graph/grapheditor.h"
#include "document.h"
#include "watchers/documentwatcher.h"
#include "watchers/logwatcher.h"
#include <zengine.h>

class ZenGarden : public QMainWindow
{
	Q_OBJECT

public:
	ZenGarden(QWidget *parent = 0);
	~ZenGarden();

private:

	/// Open viewers
	GraphEditor*				OpenGraphViewer(bool LeftPanel, GraphNode* Graph);

	/// The GL widget used for initializing OpenGL and sharing context
	QGLWidget*					CommonGLWidget;

	/// Handles Zengine log messages
	void						Log(LogMessage Message);

	//GraphEditor*				TheGraphEditor;
	Document*					Doc;
	DocumentWatcher*			DocWatcher;
	UINT						NextGraphIndex;

	/// App UI
	Ui::zengardenClass ui;
	LogWatcher*					TheLogWatcher;

private slots:
	void						InitModules();
	void						DisposeModules();
	void						NewGraph();
};

#endif // ZENGARDEN_H
