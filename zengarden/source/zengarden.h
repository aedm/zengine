#pragma once

#include <QtWidgets/QMainWindow>
#include <QtOpenGL/QGLWidget>
#include "ui_zengarden.h"
#include "graph/graphwatcher.h"
#include "document.h"
#include "watchers/documentwatcher.h"
#include "watchers/logwatcher.h"
#include "propertyeditor/propertyeditor.h"
#include <zengine.h>

class ZenGarden: public QMainWindow {
  Q_OBJECT

public:
  ZenGarden(QWidget *parent = 0);
  ~ZenGarden();

private:

  /// Open viewers
  GraphWatcher* OpenGraphViewer(bool leftPanel, GraphNode* graph);
  void Watch(Node* node, WatcherWidget* widget);
  void SetNodeForPropertyEditor(Node* node);

  /// Test code
  Texture* CreateSampleTexture();

  /// The GL widget used for initializing OpenGL and sharing context
  QGLWidget* mCommonGLWidget;

  Document*	mDocument;
  DocumentWatcher* mDocumentWatcher;
  UINT mNextGraphIndex;

  /// App UI
  Ui::zengardenClass mUI;
  LogWatcher*	mLogWatcher;

  //PropertyEditor*				PropEditor;
  WatcherWidget* mPropertyWatcher;
  QBoxLayout* mPropertyLayout;

private slots:
  void InitModules();
  void DisposeModules();
  void NewGraph();
};
