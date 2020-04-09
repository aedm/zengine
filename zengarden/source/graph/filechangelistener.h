#pragma once

#include <zengine.h>
#include <QDir>
#include <QFileSystemWatcher>
#include <memory>

// Listens for changes in the file system relevant to the project
class FileChangeListener: public QObject {
  Q_OBJECT

public:
  FileChangeListener();

  void SetProjectDirectory(const QString& projectDirectory);

private:
  QFileSystemWatcher mFileWatcher;
  QString mProjectDir;

private slots:
  void FileChanged(const QString& fileName);
  void DirectoryChanged(const QString& directoryName);

  void HandleProjectDirectoryChange();
  void HandleGraphDirectoryChange(const QString& directoryName);
};


