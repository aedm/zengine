#pragma once

#include <QFileSystemWatcher>
#include "ui_zengarden.h"
#include "watchers/documentwatcher.h"
#include "watchers/logwatcher.h"
#include "watchers/watcherwidget.h"
#include <zengine.h>
#include <QtCore/QTime>
#include <QtCore/QDir>

class ZenGarden: public QMainWindow {
  Q_OBJECT

public:
  ZenGarden(QWidget *parent = nullptr);
  ~ZenGarden();

  static ZenGarden* GetInstance();
  static void OpenGLMakeCurrent();

  /// Open a watcher
  void Watch(const std::shared_ptr<Node>& node, WatcherPosition watcherPosition);

  /// Property editor related
  void SetNodeForPropertyEditor(const std::shared_ptr<Node>& node);
  std::shared_ptr<Node> GetNodeInPropertyEditor() const;

  /// Sets the cursor relative to the beginning of the timeline
  void SetMovieCursor(float beats);

  /// Returns the global time in seconds
  float GetGlobalTime() const;

  /// Returns the movie cursor position in seconds
  float GetMovieCursor() const;

  /// Event that fires when the movie cursor changes
  Event<float> mOnMovieCursorChange;

  /// Returns the PropertiesNode associated with the current document
  std::shared_ptr<PropertiesNode> GetPropertiesNode() const;

  /// Returns the PropertiesNode associated with the current document
  std::shared_ptr<MovieNode> GetMovieNode() const;

private:
  /// Closes a watcher tab
  void DeleteWatcherWidget(WatcherWidget* widget);

  void SetupMovieWatcher();

  WatcherWidget* mPropertyEditor = nullptr;
  QBoxLayout* mPropertyLayout = nullptr;

  WatcherWidget* mMovieWatcherWidget = nullptr;
  QBoxLayout* mMovieWatcherLayout = nullptr;

  /// The GL widget used for initializing OpenGL and sharing context
  QGLWidget* mCommonGLWidget = nullptr;

  /// The currently open document
  std::shared_ptr<Document> mDocument;
  DocumentWatcher* mDocumentWatcher = nullptr;
  QString mDocumentDirectory;
  
  /// When creating a new Graph, this number will be its index
  UINT mNextGraphIndex = 0;

  /// App UI
  Ui::zengardenClass mUI{};
  LogWatcher*	mLogWatcher = nullptr;

  /// Global time elapsed since app launch
  QTime mTime;
  float GetElapsedBeats() const;

  /// Global time when movie started playing in milliseconds
  float mMovieStartBeat = 0.0f;

  /// Current movie cursor position in beats, updates when movie is playing
  float mMovieCursor = 0.0f;

  bool mPlayMovie = false;
  void RestartMovieTimer();

  /// Engine shaders
  void LoadEngineShaders(const QDir& dir);
  QDir mEngineShadersDir;
  QFileSystemWatcher mEngineShadersFolderWatcher;

  void keyPressEvent(QKeyEvent* event) override;

  /// Music related
  int mBassMusicChannel = 0;
  void LoadMusic();
  void PlayMusic(float beats) const;
  void StopMusic() const;

private slots:
  void InitModules();
  void DisposeModules();
  void DeleteDocument();

  /// Handles the change of time
  void Tick();

  /// Loads an engine-level shader file
  void LoadEngineShader(const QString& path) const;

  /// Menu buttons
  void HandleMenuSaveAs();
  void HandleMenuNew();
  void HandleMenuOpen();
  void HandlePropertiesMenu();
};
