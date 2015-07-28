#define _CRT_SECURE_NO_WARNINGS

#include "../serialize/json/jsonserializer.h"
#include "../serialize/json/jsondeserializer.h"
#include <include/base/helpers.h>
#include <include/base/vectormath.h>
#include <include/base/types.h>
#include <stdarg.h>


Logger* TheLogger = new Logger();

void Logger::Log2(LogSeverity severity, const wchar_t* logString, ...) {
  wchar_t tmp[4096];
  va_list args;
  va_start(args, logString);
  vswprintf(tmp, 4096, logString, args);
  va_end(args);
  onLog(LogMessage(severity, tmp));
}

void Logger::LogFunc2(LogSeverity severity, const wchar_t* function,
                      const wchar_t* logString, ...) {
  wchar_t tmp[4096];
  int funlen = swprintf(tmp, 4096, function);

  va_list args;
  va_start(args, logString);
  vswprintf(tmp + funlen, 4096 - funlen, logString, args);
  va_end(args);

  onLog(LogMessage(severity, tmp));
}

void Logger::LogFunc2(LogSeverity severity, const wchar_t* function,
                      const char* logString, ...) {
  /// God, this is bad.
  char tmp_ascii[4096];

  va_list args;
  va_start(args, logString);
  _vsnprintf(tmp_ascii, 4096, logString, args);
  va_end(args);

  string tmp_string(tmp_ascii);
  wstring tmp_wstring(tmp_string.begin(), tmp_string.end());

  wchar_t tmp[4096];
  swprintf(tmp, 4096, L"%s%s", function, tmp_wstring.c_str());

  onLog(LogMessage(severity, tmp));
}

LogMessage::LogMessage(LogSeverity severity, const wchar_t* message) {
  this->severity = severity;
  this->message = message;
}

namespace Convert {
  string IntToSring(int Value) {
    char tmp[64];
    sprintf_s(tmp, 64, "%d", Value);
    return string(tmp);
  }

  bool StringToFloat(const char* S, float& F) {
    return sscanf(S, "%f", &F) != 1;
  }
}

string ToJSON(Document* document) {
  JSONSerializer serialzer(document);
  return serialzer.GetJSON();
}

Document* FromJSON(string& json) {
  JSONDeserializer deserialzer(json);
  return deserialzer.GetDocument();
}