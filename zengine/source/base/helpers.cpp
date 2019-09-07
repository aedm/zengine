#define _CRT_SECURE_NO_WARNINGS

#include "../serialize/json/jsonserializer.h"
#include "../serialize/json/jsondeserializer.h"
#include <include/base/helpers.h>
#include <include/base/vectormath.h>
#include <stdarg.h>

Logger* TheLogger = new Logger();

static const int LogMessageMaxLength = 30000;
static wchar_t TempStringW[LogMessageMaxLength];
static char TempStringA[LogMessageMaxLength];

void Logger::Log2(LogSeverity severity, const wchar_t* logString, ...) {
  va_list args;
  va_start(args, logString);
  vswprintf(TempStringW, LogMessageMaxLength, logString, args);
  va_end(args);
  onLog(LogMessage(severity, TempStringW));
}

void Logger::LogFunc2(LogSeverity severity, const wchar_t* function,
                      const wchar_t* logString, ...) {
  int funlen = swprintf(TempStringW, LogMessageMaxLength, function);

  va_list args;
  va_start(args, logString);
  vswprintf(TempStringW + funlen, LogMessageMaxLength - funlen, logString, args);
  va_end(args);

  onLog(LogMessage(severity, TempStringW));
}

void Logger::LogFunc2(LogSeverity severity, const wchar_t* function,
                      const char* logString, ...) {
  /// God, this is bad.
  va_list args;
  va_start(args, logString);
  _vsnprintf(TempStringA, LogMessageMaxLength, logString, args);
  va_end(args);

  string tmp_string(TempStringA);
  wstring tmp_wstring(tmp_string.begin(), tmp_string.end());

  swprintf(TempStringW, LogMessageMaxLength, L"%s%s", function, tmp_wstring.c_str());

  onLog(LogMessage(severity, TempStringW));
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

  std::wstring StringToWstring(const string& s) {
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
  }
}

string ToJSON(const shared_ptr<Document>& document) {
  JSONSerializer serialzer(document);
  return serialzer.GetJSON();
}

shared_ptr<Document> FromJSON(string& json) {
  JSONDeserializer deserialzer(json);
  return deserialzer.GetDocument();
}