#include "filechangelistener.h"
#include <QDirIterator>

FileChangeListener::FileChangeListener() {
  connect(&mFileWatcher, SIGNAL(fileChanged(const QString&)),
    this, SLOT(FileChanged(const QString&)));
  connect(&mFileWatcher, SIGNAL(directoryChanged(const QString&)),
    this, SLOT(DirectoryChanged(const QString&)));
}

void FileChangeListener::SetProjectDirectory(const QString& projectDirectory) {
  INFO("Watching project directory '%s'", projectDirectory.toStdString().c_str());
  mProjectDir = projectDirectory;
  mFileWatcher.addPath(projectDirectory);
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
  for (auto dir : dirs) {
    INFO(dir.toStdString().c_str());
    mFileWatcher.addPath(QDir(mProjectDir).filePath(dir));
  }
}

void FileChangeListener::HandleGraphDirectoryChange(const QString& directoryName) {
  INFO("GRAPH %s", directoryName.toStdString().c_str());
}
