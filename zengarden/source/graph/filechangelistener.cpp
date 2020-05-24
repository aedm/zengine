#include "filechangelistener.h"
#include "../util/util.h"
#include <QtCore/QDirIterator>
#include <vector>

using std::vector;

FileChangeListener::FileChangeListener() {
  connect(&mFileWatcher, SIGNAL(fileChanged(const QString&)),
    this, SLOT(FileChanged(const QString&)));
  connect(&mFileWatcher, SIGNAL(directoryChanged(const QString&)),
    this, SLOT(DirectoryChanged(const QString&)));
}

void FileChangeListener::SetProjectDirectory(const QString& projectDirectory,
  shared_ptr<Document>& document)
{
  /// Reset state
  mFileWatcher.removePaths(mFileWatcher.files());
  mFileWatcher.removePaths(mFileWatcher.directories());
  mPathsToGraphs.clear();
  mPathsToNodes.clear();
  mProjectDir.clear();
  mDocument.reset();

  /// Add new directory
  INFO("Watching project directory '%s'", projectDirectory.toStdString().c_str());
  mProjectDir = projectDirectory;
  mDocument = document;
  mFileWatcher.addPath(projectDirectory);
  HandleProjectDirectoryChange();
}

void FileChangeListener::FileChanged(const QString& fileName) {
  INFO("File changed: %s", fileName.toStdString().c_str());
}

void FileChangeListener::DirectoryChanged(const QString& directoryName) {
  INFO("Directory changed: %s", directoryName.toStdString().c_str());
  if (directoryName == mProjectDir) {
    HandleProjectDirectoryChange();
    return;
  }
  HandleGraphDirectoryChange(directoryName);
}

void FileChangeListener::HandleProjectDirectoryChange() {
  QStringList dirs = QDir(mProjectDir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);

  /// Collect changes
  auto dirList = dirs.toStdList();
  vector<QString> dirVector(dirList.begin(), dirList.end());
  vector<QString> newDirs, deletedDirs;
  Util::CollectChanges(mPathsToGraphs, dirVector, newDirs, deletedDirs);

  /// Handle newly added graphs
  for (auto& dir : newDirs) {
    INFO("Loading graph directory '%s'", dir.toStdString().c_str());
    /// New graph, add it to the list
    const shared_ptr<Graph> graph = std::make_shared<Graph>();
    graph->SetName(dir.toStdString());
    mDocument->mGraphs.Connect(graph);
    mPathsToGraphs[dir] = graph;
    mFileWatcher.addPath(QDir(mProjectDir).filePath(dir));
    HandleGraphDirectoryChange(dir);
  }

  /// Handle removed graphs
  for (auto& dir : deletedDirs) {
    INFO("Removed graph directory '%s'", dir.toStdString().c_str());
    shared_ptr<Graph> graph = mPathsToGraphs.at(dir);
    //mDocument->mGraphs.Disconnect(graph);
    graph->Dispose();
  }
}

void FileChangeListener::HandleGraphDirectoryChange(const QString& directoryName) {
  INFO("GRAPH %s", directoryName.toStdString().c_str());
}
