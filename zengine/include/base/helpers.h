#pragma once
#include "system.h"
#include <memory>

using namespace std;

#if 1
#	ifdef _DEBUG
#		define ASSERT(x) { if (!(x)) __debugbreak(); }
#		define DEBUGBREAK(errstr) { ERR(errstr); __debugbreak(); }
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
// ReSharper disable CppConstValueFunctionReturnType
const shared_ptr<T> PointerCast(const shared_ptr<K>& object) {
  // ReSharper restore CppConstValueFunctionReturnType
#ifdef _DEBUG
  const shared_ptr<T> cast = dynamic_pointer_cast<T>(object);
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
template<typename CharType>
struct EnumMapper {
  const CharType*	name;
  int	value;

  static const CharType* GetStringFromEnum(const EnumMapper<CharType>* mapper,
                                           int enumId);
  static int GetEnumFromString(const EnumMapper<CharType>* mapper,
                               const CharType* value);
  static int GetEnumFromString(const EnumMapper<CharType>* mapper,
                               const CharType* value, int stringLength);
};

typedef EnumMapper<char> EnumMapperA;
typedef EnumMapper<wchar_t>	EnumMapperW;


template<typename CharType>
const CharType*	EnumMapper<CharType>::GetStringFromEnum(
    const EnumMapper<CharType>* mapper, int enumId) {
  for (; mapper->value != -1; ++mapper) {
    if (enumId == mapper->value) return mapper->name;
  }
  ERR(L"GetStringFromEnum: Unknown enum: %d.", enumId);
  return nullptr;
}


template<typename CharType>
int EnumMapper<CharType>::GetEnumFromString(const EnumMapper<CharType>* mapper, 
                                            const CharType* value) {
  for (; mapper->value != -1; ++mapper) {
    if (strcmp(value, mapper->name) == 0) return mapper->value;
  }
  return -1;
}

template<typename CharType>
int EnumMapper<CharType>::GetEnumFromString(const EnumMapper<CharType>* mapper, 
                                            const CharType* value, int stringLength) {
  for (; mapper->value != -1; ++mapper) {
    if (strlen(mapper->name) == stringLength &&
        strncmp(value, mapper->name, stringLength) == 0) {
      return mapper->value;
    }
  }
  return -1;
}

namespace Convert {
  string IntToString(int value);
  bool StringToFloat(const char* s, float& f);
  wstring StringToWstring(const string& sourceString);
}

string ToJSON(const shared_ptr<Document>& document);
shared_ptr<Document> FromJSON(string& json);
