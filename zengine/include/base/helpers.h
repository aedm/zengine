#pragma once
#include "system.h"
#include <assert.h>

#if 1
#	ifdef _DEBUG
#		define ASSERT(x) { if (!(x)) __debugbreak(); }
#	else
#		define ASSERT(x)
#	endif
#else
#	define ASSERT assert
#endif

#define NOT_IMPLEMENTED ASSERT(false);
#define SHOULDNT_HAPPEN ASSERT(false);

/// Logging facility 

class Logger;
extern Logger* TheLogger;

#define MAGIC(x) MAGIC2(x)
#define MAGIC2(x) #x
#define LOGHEADER L"[" _STR2WSTR(__FUNCTION__) L"():" _STR2WSTR(MAGIC(__LINE__)) L"]: "
//#define LOGHEADER L"[" _STR2WSTR(__FILE__) L":" _STR2WSTR(MAGIC(__LINE__)) L"] " _STR2WSTR(__FUNCTION__) L"(): "
//#define LOGHEADER _STR2WSTR(__FUNCTION__) L"(): "

#define LOG(severity, message, ...) TheLogger->LogFunc2(severity, LOGHEADER, message, __VA_ARGS__)
#define INFO(message, ...) TheLogger->LogFunc2(LOG_INFO, LOGHEADER, message, __VA_ARGS__)
#define WARN(message, ...) TheLogger->LogFunc2(LOG_WARNING, LOGHEADER, message, __VA_ARGS__)
#define ERR(message, ...) TheLogger->LogFunc2(LOG_ERROR, LOGHEADER, message, __VA_ARGS__)

enum LogSeverity 
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

struct LogMessage
{
	LogMessage(LogSeverity Severity, const wchar_t* Message);

	LogSeverity				Severity;
	const wchar_t*			Message;
};

class Logger
{
public:
	/// Logging
	void					Log2(LogSeverity Severity, const wchar_t* LogString, ...);
	void					LogFunc2(LogSeverity Severity, const wchar_t* Function, const wchar_t* LogString, ...);
	void					LogFunc2(LogSeverity Severity, const wchar_t* Function, const char* LogString, ...);

	/// Events
	Event<LogMessage>		OnLog;
};


/// Enum mapping facility 

template<typename CharType>
struct EnumMapper
{
	const CharType*			Name;
	int						Value;

	static const CharType*	GetStringFromEnum(const EnumMapper<CharType>* Mapper, int EnumId);
	static int				GetEnumFromString(const EnumMapper<CharType>* Mapper, const CharType* Value);
	static int				GetEnumFromString(const EnumMapper<CharType>* Mapper, const CharType* Value, int StringLength);
};

typedef EnumMapper<char>	EnumMapperA;
typedef EnumMapper<wchar_t>	EnumMapperW;


template<typename CharType>
const CharType*	EnumMapper<CharType>::GetStringFromEnum(const EnumMapper<CharType>* Mapper, int EnumId)
{
	for (const EnumMapper* mapper = Mapper; mapper->Value != -1; mapper++)
	{
		if (EnumId == mapper->Value) return mapper->Name;
	}
	ERR(L"GetStringFromEnum: Unknown enum: %d.", EnumId);
	return NULL;
}


template<typename CharType>
int EnumMapper<CharType>::GetEnumFromString(const EnumMapper<CharType>* Mapper, const CharType* Value)
{
	for (const EnumMapper* mapper = Mapper; mapper->Value != -1; mapper++)
	{
		if (strcmp(Value, mapper->Name) == 0) return mapper->Value;
	}
	return -1;
}

template<typename CharType>
int EnumMapper<CharType>::GetEnumFromString(const EnumMapper<CharType>* Mapper, const CharType* Value, int StringLength)
{
	for (const EnumMapper* mapper = Mapper; mapper->Value != -1; mapper++)
	{
		if (strlen(mapper->Name) == StringLength && 
			strncmp(Value, mapper->Name, StringLength) == 0)
		{
			return mapper->Value;
		}
	}
	return -1;
}

namespace Convert
{
	string			IntToSring(int Value);
	bool			StringToFloat(const char* S, float& F);
}
