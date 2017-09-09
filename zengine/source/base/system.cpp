#include <include/base/system.h>
#include <include/base/helpers.h>
#include <io.h>
#include <direct.h> 

char* System::ReadFile(const wchar_t* fileName) {
  FILE* file = _wfopen(fileName, L"rb");
  if (!file) {
    ERR(L"Cannot open file for read: %s", fileName);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  unsigned __int64 fileLength = _ftelli64(file);
  fseek(file, 0, SEEK_SET);

  char* fileContent = new char[(unsigned int)fileLength + 1];
  fileContent[fileLength] = 0;

  if (fread(fileContent, 1, (size_t)fileLength, file) != fileLength) {
    delete fileContent;
    ERR(L"Cannot read file: %s", fileName);
    return NULL;
  }
  fclose(file);

  INFO(L"Reading file done: %s", fileName);
  return fileContent;
}

void System::ReadFilesInFolder(const wchar_t* folder, const wchar_t* filter,
                               vector<wstring>& oFileList) {
  wchar_t currentDir[1024];
  _wgetcwd(currentDir, 1023);

  int err = _wchdir(folder);

  _wfinddata_t fileInfo;
  auto handle = _wfindfirst(filter, &fileInfo);
  if (handle == -1) {
    ERR(L"Cannot read folder: %s", folder);
    return;
  }

  for (auto ret = 0; ret == 0; ret = _wfindnext(handle, &fileInfo)) {
    wstring fullName(folder);
    fullName.append(fileInfo.name);
    oFileList.push_back(fullName);
  }

  _findclose(handle);
  _wchdir(currentDir);
}