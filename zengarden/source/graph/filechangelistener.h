#pragma once

#include <zengine.h>
#include <QtCore/QDir>
#include <QtCore/QFileSystemWatcher>
#include <memory>

using std::map;
using std::shared_ptr;

// Listens for changes in the file system relevant to the project
class FileChangeListener: public QObject {
  Q_OBJECT

public:
  FileChangeListener();

  void SetProjectDirectory(const QString& projectDirectory, 
    shared_ptr<Document>& document);

private:
  QFileSystemWatcher mFileWatcher;
  QString mProjectDir;
  shared_ptr<Document> mDocument;

  map<QString, shared_ptr<Node>> mPathsToNodes;
  map<QString, shared_ptr<Graph>> mPathsToGraphs;

private slots:
  void FileChanged(const QString& fileName);
  void DirectoryChanged(const QString& directoryName);

  void HandleProjectDirectoryChange();
  void HandleGraphDirectoryChange(const QString& directoryName);
};


