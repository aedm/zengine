#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "fastdelegate.h"
#include "defines.h"
#include <set>
#include <vector>

using namespace std;
using namespace fastdelegate;

class System
{
public:
	static OWNERSHIP char*	ReadFile(const wchar_t* FileName);

	static void				ReadFilesInFolder(const wchar_t* Folder, const wchar_t* Extension, vector<wstring>& oFileList);
};

template <typename T>
class Event;

template <>
class Event<void>
{
public:
	void operator += (const FastDelegate0<void>& Delegate)
	{
		Delegates.insert(Delegate);
	}

	void operator -= (const FastDelegate0<void>& Delegate)
	{
		Delegates.erase(Delegate);
	}

	void operator ()()
	{
		for (set<const FastDelegate0<void>>::iterator it = Delegates.begin(); it != Delegates.end(); ++it)
		{
			(*it)();
		}
	}

	set<const FastDelegate0<void>>	Delegates;	
};

template <typename T>
class Event
{
public:
	void operator += (const FastDelegate1<T, void>& Delegate)
	{
		Delegates.insert(Delegate);
	}

	void operator -= (const FastDelegate1<T, void>& Delegate)
	{
		Delegates.erase(Delegate);
	}

	void operator () (T Message)
	{
		for (set<const FastDelegate1<T, void>>::iterator it = Delegates.begin(); it != Delegates.end(); ++it)
		{
			(*it)(Message);
		}
	}

	set<const FastDelegate1<T, void>>	Delegates;
};
