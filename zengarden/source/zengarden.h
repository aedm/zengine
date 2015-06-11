#ifndef ZENGARDEN_H
#define ZENGARDEN_H

#include <QtWidgets/QMainWindow>
#include <QtOpenGL/QGLWidget>
#include "ui_zengarden.h"
#include "graph/grapheditor.h"
#include "document.h"
#include "watchers/documentwatcher.h"
#include "watchers/logwatcher.h"
#include "propertyeditor/propertyeditor.h"
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
	void						Watch(Node* Nd, WatcherWidget* Widget);
	void						SetNodeForPropertyEditor(Node* Nd);

	/// The GL widget used for initializing OpenGL and sharing context
	QGLWidget*					CommonGLWidget;

	//GraphEditor*				TheGraphEditor;
	Document*					Doc;
	DocumentWatcher*			DocWatcher;
	UINT						NextGraphIndex;

	/// App UI
	Ui::zengardenClass ui;
	LogWatcher*					TheLogWatcher;

	//PropertyEditor*				PropEditor;
  WatcherWidget* mPropertyWatcher;
  QBoxLayout* mPropertyLayout;

private slots:
	void						InitModules();
	void						DisposeModules();
	void						NewGraph();
};

#endif // ZENGARDEN_H
