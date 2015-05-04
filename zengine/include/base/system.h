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

template <class X, class Y, class RetType, typename... Params>
FastDelegate<RetType(Params...)> Delegate(Y* x, RetType(X::*func)(Params...)) {
	return FastDelegate<RetType(Params...)>(x, func);
}

template <class X, class Y, class RetType, typename... Params>
FastDelegate<RetType(Params...)> Delegate(Y* x, RetType(X::*func)(Params...) const) {
	return FastDelegate<RetType(Params...)>(x, func);
}

template <typename... Types>
class Event
{
public:
	void operator += (const FastDelegate<void(Types...)>& Delegate)
	{
		Delegates.insert(Delegate);
	}

	void operator -= (const FastDelegate<void(Types...)>& Delegate)
	{
		Delegates.erase(Delegate);
	}

	void operator () (Types... Message)
	{
		for (set<const FastDelegate<void(Types...)>>::iterator it = Delegates.begin(); it != Delegates.end(); ++it)
		{
			(*it)(Message...);
		}
	}

	set<const FastDelegate<void(Types...)>>	Delegates;
};
