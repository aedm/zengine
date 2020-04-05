#pragma once
#include "system.h"
#include <memory>
#include <string_view>

using std::string_view;

#if 1
#	ifdef _DEBUG
#		define ASSERT(x) { if (!(x)) __debugbreak(); }
#		define DEBUGBREAK __debugbreak()
#	else
#		define ASSERT(x)
#		define DEBUGBREAK
#	endif
#else
#	define ASSERT assert
#endif

#define NOT_IMPLEMENTED ASSERT(false);
#define SHOULD_NOT_HAPPEN ASSERT(false);

template<typename T, typename K>
T SafeCast(K object) {
  #ifdef _DEBUG
  T cast = dynamic_cast<T>(object);
  ASSERT(cast != nullptr);
  return cast;
  #else 
  return static_cast<T>(object);
  #endif
}

template<typename T, typename K>
// ReSharper disable once CppConstValueFunctionReturnType
const std::shared_ptr<T> PointerCast(const std::shared_ptr<K>& object) {
#ifdef _DEBUG
  const std::shared_ptr<T> cast = std::dynamic_pointer_cast<T>(object);
  ASSERT(object == nullptr || cast != nullptr);
  return cast;
#else 
  return static_pointer_cast<T>(object);
#endif
}

template<typename T, typename K>
T SafeCastAllowNull(K object) {
#ifdef _DEBUG
  T cast = dynamic_cast<T>(object);
  ASSERT(object == nullptr || cast != nullptr);
  return cast;
#else 
  return static_cast<T>(object);
#endif
}

/// Logging facility 
class Document;
class Logger;
extern Logger* TheLogger;

#define __STR2WSTR(str) L##str
#define _STR2WSTR(str) __STR2WSTR(str)

#define MAGIC(x) MAGIC2(x)
#define MAGIC2(x) #x
#define LOGHEADER \
	  L"[" _STR2WSTR(__FUNCTION__) L"():" _STR2WSTR(MAGIC(__LINE__)) L"]: "

#define LOG(severity, message, ...) \
    TheLogger->LogFunc2(severity, LOGHEADER, message, __VA_ARGS__)
#define INFO(message, ...) \
    TheLogger->LogFunc2(LOG_INFO, LOGHEADER, message, __VA_ARGS__)
#define WARN(message, ...) \
    TheLogger->LogFunc2(LOG_WARNING, LOGHEADER, message, __VA_ARGS__)
#define ERR(message, ...) \
    TheLogger->LogFunc2(LOG_ERROR, LOGHEADER, message, __VA_ARGS__)

enum LogSeverity {
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR
};

struct LogMessage {
  LogMessage(LogSeverity severity, const wchar_t* message);

  LogSeverity	severity;
  const wchar_t* message;
};

class Logger {
public:
  /// Logging
  void Log2(LogSeverity severity, const wchar_t* logString, ...);

  void LogFunc2(LogSeverity severity, const wchar_t* function,
                const wchar_t* logString, ...);

  void LogFunc2(LogSeverity severity, const wchar_t* function,
                const char* logString, ...);

  /// Events
  Event<LogMessage> onLog;
};


/// Enum mapping facility 
template <typename Enum, typename Name>
class EnumMap
{
public:
  struct Item {
    Name mName;
    Enum mEnum;
  };
  EnumMap(const std::initializer_list<Item>& items) : mItems(items) {}
  Enum GetEnum(Name name) const {
    for (const auto& item : mItems) {
      if (strcmp(item.mName, name) == 0) return item.mEnum;
    }
    return Enum(-1);
  }
  Name GetName(Enum enumValue) const {
    for (const auto& item : mItems) {
      if (item.mEnum == enumValue) return item.mName;
    }
    return nullptr;
  }
  Enum GetEnumA(const string_view& name) const {
    for (const auto& item : mItems) {
      if (name == item.mName) {
        return item.mEnum;
      }
    }
    return Enum(-1);
  }
  std::vector<Item> mItems;
};

template <typename Enum> using EnumMapA = EnumMap<Enum, const char*>;

namespace Convert {
  std::wstring StringToWstring(const std::string& sourceString);
  std::string WstringToString(const std::wstring& sourceString);
}

std::string ToJson(const std::shared_ptr<Document>& document);
std::shared_ptr<Document> FromJson(const std::string& json);
std::string GenerateNodeId();