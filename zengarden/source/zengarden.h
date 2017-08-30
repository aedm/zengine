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
#include <QtCore/QDir>
#include <bass.h>

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
  Node* GetNodeInPropertyEditor();

  /// Sets the cursor relative to the beginning of the timeline
  void SetMovieCursor(float seconds);

  /// Sets the cursor relative to the beginning of the current clip
  void SetClipCursor(float seconds);

  /// Returns the global time in seconds
  float GetGlobalTime();

  /// Returns the movie cursor position in seconds
  float GetMovieCursor();

  void SetSceneNodeForClip(SceneNode* sceneNode);

  /// Event that fires when the movie cursor changes
  Event<float> mOnMovieCursorChange;

  /// Returns the PropertiesNode associated with the current document
  PropertiesNode* GetPropertiesNode();

private:
  /// Closes a watcher tab
  void DeleteWatcherWidget(WatcherWidget* widget);

  /// Creates blank document
  void CreateNewDocument();

  void SetupMovieWatcher();

  WatcherWidget* mPropertyEditor = nullptr;
  QBoxLayout* mPropertyLayout = nullptr;

  WatcherWidget* mMovieWatcherWidget = nullptr;
  QBoxLayout* mMovieWatcherLayout = nullptr;

  /// The GL widget used for initializing OpenGL and sharing context
  QGLWidget* mCommonGLWidget = nullptr;

  /// The currently open document
  Document*	mDocument = nullptr;
  DocumentWatcher* mDocumentWatcher = nullptr;
  QString mDocumentFileName;
  
  /// When creating a new Graph, this number will be its index
  UINT mNextGraphIndex;

  /// App UI
  Ui::zengardenClass mUI;
  LogWatcher*	mLogWatcher = nullptr;

  /// Global time elapsed since app launch
  QTime mTime;

  /// Global time when movie started playing in milliseconds
  int mMovieStartTime;

  /// Current movie cursor position in seconds, updates when movie is playing
  float mMovieCursor;

  bool mPlayMovie = false;
  void RestartMovieTimer();

  /// Engine shaders
  void LoadEngineShaders(QDir& dir);
  QDir mEngineShadersDir;
  QFileSystemWatcher mEngineShadersFolderWatcher;

  virtual void keyPressEvent(QKeyEvent* event) override;

  /// Music related
  DWORD mBassMusicChannel = -1;
  void LoadMusic();
  void PlayMusic(float seconds);
  void StopMusic();

private slots:
  void InitModules();
  void DisposeModules();
  void DeleteDocument();

  /// Handles the change of time
  void Tick();

  /// Loads an engine-level shader file
  void LoadEngineShader(const QString& path);

  /// Menu buttons
  void HandleMenuSaveAs();
  void HandleMenuNew();
  void HandleMenuOpen();
  void HandlePropertiesMenu();
};
