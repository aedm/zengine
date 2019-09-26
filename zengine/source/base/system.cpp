#include <include/base/system.h>
#include <include/base/helpers.h>
#include <io.h>
#include <direct.h> 

char* System::ReadFile(const wchar_t* fileName) {
  FILE* file = _wfopen(fileName, L"rb");
  if (!file) {
    ERR(L"Cannot open file for read: %s", fileName);
    return nullptr;
  }

  fseek(file, 0, SEEK_END);
  const unsigned __int64 fileLength = _ftelli64(file);
  fseek(file, 0, SEEK_SET);

  char* fileContent = new char[static_cast<unsigned int>(fileLength) + 1];
  fileContent[fileLength] = 0;

  if (fread(fileContent, 1, static_cast<size_t>(fileLength), file) != 
    fileLength) {
    delete[] fileContent;
    ERR(L"Cannot read file: %s", fileName);
    return nullptr;
  }
  fclose(file);

  INFO(L"Reading file done: %s", fileName);
  return fileContent;
}

void System::ReadFilesInFolder(const wchar_t* folder, const wchar_t* extension,
                               vector<wstring>& oFileList) {
  wchar_t currentDir[1024];
  _wgetcwd(currentDir, 1023);

  int err = _wchdir(folder);

  _wfinddata_t fileInfo;
  const auto handle = _wfindfirst(extension, &fileInfo);
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