#include <include/base/system.h>
#include <include/base/helpers.h>
#include <io.h>
#include <direct.h> 

char* System::ReadFile(const wchar_t* FileName)
{
	FILE* file = _wfopen(FileName, L"rb");
	if (!file)
	{
		ERR(L"Cannot open file for read: %s", FileName);
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	unsigned __int64 fileLength = _ftelli64(file);
	fseek(file, 0, SEEK_SET);

	char* fileContent = new char[(unsigned int)fileLength+1];
	fileContent[fileLength] = 0;
	
	if (fread(fileContent, 1, (size_t)fileLength, file) != fileLength)
	{
		delete fileContent;
		ERR(L"Cannot read file: %s", FileName);
		return NULL;
	}
	fclose(file);

	INFO(L"Reading file done: %s", FileName);
	return fileContent;
}

void System::ReadFilesInFolder( const wchar_t* Folder, const wchar_t* Filter, vector<wstring>& oFileList )
{
	wchar_t currentDir[512];
	_wgetcwd(currentDir, 511);

	_wchdir(Folder);

	_wfinddata_t fileInfo;
	long handle = _wfindfirst(Filter, &fileInfo);
	if (handle == -1)
	{
		ERR(L"Cannot read folder: %s", Folder);
		return;
	}

	for (long ret = 0; ret == 0; ret = _wfindnext(handle, &fileInfo))
	{
		wstring fullName(Folder);
		fullName.append(fileInfo.name);
		oFileList.push_back(fullName);
	} 

	_findclose(handle);
	_wchdir(currentDir);
}