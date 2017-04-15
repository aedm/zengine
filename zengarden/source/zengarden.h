#pragma once

#include <QtWidgets/QMainWindow>
#include <QtOpenGL/QGLWidget>
#include <QFileSystemWatcher>
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

  static ZenGarden* GetInstance();

  /// Open a watcher
  void Watch(Node* node, WatcherPosition watcherPosition);

  /// Property editor related
  void SetNodeForPropertyEditor(Node* node);

  /// Closes a watcher tab
  void DeleteWatcherWidget(WatcherWidget* widget);

private:

  WatcherWidget* mPropertyEditor;
  QBoxLayout* mPropertyLayout;

  /// The GL widget used for initializing OpenGL and sharing context
  QGLWidget* mCommonGLWidget;

  /// The currently open document
  Document*	mDocument;
  DocumentWatcher* mDocumentWatcher;
  QString mDocumentFileName;
  
  /// When creating a new Graph, this number will be its index
  UINT mNextGraphIndex;

  /// App UI
  Ui::zengardenClass mUI;
  LogWatcher*	mLogWatcher;

  /// Global time
  QTime mTime;
  int mSceneStartTime;
  bool mPlayScene = false;
  void RestartSceneTimer();

  /// Engine shaders
  void LoadEngineShaders(QString& path);
  QFileSystemWatcher mEngineShadersFolderWatcher;

  virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
  void InitModules();
  void DisposeModules();
  void NewGraph();
  void UpdateTimeNode();
  void DeleteDocument();

  /// Loads an engine-level shader file
  void LoadEngineShader(const QString& path);

  /// Menu buttons
  void HandleMenuSaveAs();
  void HandleMenuNew();
  void HandleMenuOpen();
};
