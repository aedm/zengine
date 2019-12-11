#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include "../serialize/json/jsonserializer.h"
#include "../serialize/json/jsondeserializer.h"
#include <include/base/helpers.h>
#include <cstdarg>
#include <codecvt>

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
  const int funlen = swprintf(TempStringW, LogMessageMaxLength, function);

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

  std::string tmp_string(TempStringA);
  const std::wstring tmp_wstring(tmp_string.begin(), tmp_string.end());

  swprintf(TempStringW, LogMessageMaxLength, L"%s%s", function, tmp_wstring.c_str());

  onLog(LogMessage(severity, TempStringW));
}

LogMessage::LogMessage(LogSeverity severity, const wchar_t* message) {
  this->severity = severity;
  this->message = message;
}

namespace Convert {
  std::wstring StringToWstring(const std::string& s) {
    std::wstring temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
  }
  std::string WstringToString(const std::wstring& s) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(s);
  }
}

std::string ToJson(const std::shared_ptr<Document>& document) {
  const JSONSerializer serializer(document);
  return serializer.GetJSON();
}

std::shared_ptr<Document> FromJson(const std::string& json) {
  const JSONDeserializer deserializer(json);
  return deserializer.GetDocument();
}