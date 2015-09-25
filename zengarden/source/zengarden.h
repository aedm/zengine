#pragma once

#include <QtWidgets/QMainWindow>
#include <QtOpenGL/QGLWidget>
#include "ui_zengarden.h"
#include "graph/graphwatcher.h"
#include "watchers/documentwatcher.h"
#include "watchers/logwatcher.h"
#include "propertyeditor/propertyeditor.h"
#include <zengine.h>
#include <QtCore/QTime>


class ZenGarden: public QMainWindow {
  Q_OBJECT

public:
  ZenGarden(QWidget *parent = 0);
  ~ZenGarden();

private:
  /// Open and close viewers
  void Watch(Node* node, WatcherWidget* widget);
  void CloseWatcherTab(WatcherWidget* widget);

  /// Property editor related
  void SetNodeForPropertyEditor(Node* node);
  void RemovePropertyEditor(WatcherWidget* watcherWidget);
  WatcherWidget* mPropertyEditor;
  QBoxLayout* mPropertyLayout;

  /// The GL widget used for initializing OpenGL and sharing context
  QGLWidget* mCommonGLWidget;

  Document*	mDocument;
  DocumentWatcher* mDocumentWatcher;
  UINT mNextGraphIndex;

  /// App UI
  Ui::zengardenClass mUI;
  LogWatcher*	mLogWatcher;

  QTime mTime;

private slots:
  void InitModules();
  void DisposeModules();
  void NewGraph();

  void UpdateTimeNode();

  void DeleteDocument();

  /// Menu buttons
  void HandleMenuSaveAs();
  void HandleMenuNew();
  void HandleMenuOpen();
};
