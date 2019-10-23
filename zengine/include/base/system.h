#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "fastdelegate.h"
#include "defines.h"
#include <set>
#include <vector>

using namespace fastdelegate;

class System {
public:
  static OWNERSHIP char* ReadFile(const wchar_t* fileName);

  static void ReadFilesInFolder(const wchar_t* folder, const wchar_t* extension, 
                                std::vector<std::wstring>& oFileList);
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
class Event {
public:
  void operator += (const FastDelegate<void(Types...)>& delegate) {
    mDelegates.insert(delegate);
  }

  void operator -= (const FastDelegate<void(Types...)>& delegate) {
    mDelegates.erase(delegate);
  }

  void operator () (Types... message) {
    for (auto& delegate : mDelegates) {
      delegate(message...);
    }
  }

  std::set<FastDelegate<void(Types...)>>	mDelegates;
};
