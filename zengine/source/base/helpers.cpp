#include <include/base/helpers.h>
#include <include/base/vectormath.h>
#include <include/base/types.h>
#include <stdarg.h>


Logger* TheLogger = new Logger();

void Logger::Log2( LogSeverity Severity, const wchar_t* LogString, ... )
{
	wchar_t tmp[4096];
	va_list args;
	va_start(args, LogString);
	vswprintf(tmp, 4096, LogString, args);
	va_end(args);
	OnLog(LogMessage(Severity, tmp));
}

void Logger::LogFunc2(LogSeverity Severity, const wchar_t* Function, const wchar_t* LogString, ...)
{
	wchar_t tmp[4096];
	int funlen = swprintf(tmp, 4096, Function);

	va_list args;
	va_start(args, LogString);
	vswprintf(tmp + funlen, 4096 - funlen, LogString, args);
	va_end(args);

	OnLog(LogMessage(Severity, tmp));
}

void Logger::LogFunc2(LogSeverity Severity, const wchar_t* Function, const char* LogString, ...)
{
	/// God, this is bad.
	char tmp_ascii[4096];

	va_list args;
	va_start(args, LogString);
	_vsnprintf(tmp_ascii, 4096, LogString, args);
	va_end(args);

	string tmp_string(tmp_ascii);
	wstring tmp_wstring(tmp_string.begin(), tmp_string.end());

	wchar_t tmp[4096]; 
	swprintf(tmp, 4096, L"%s%s", Function, tmp_wstring.c_str());

	OnLog(LogMessage(Severity, tmp));
}

LogMessage::LogMessage( LogSeverity Severity, const wchar_t* Message )
{
	this->Severity = Severity;
	this->Message = Message;
}

namespace Convert
{
	string IntToSring( int Value )
	{
		char tmp[64];
		sprintf_s(tmp, 64, "%d", Value);
		return string(tmp);	
	}

	bool StringToFloat( const char* S, float& F )
	{
		return sscanf(S, "%f", &F) != 1;
	}
}
