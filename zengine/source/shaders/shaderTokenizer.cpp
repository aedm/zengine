#include "shaderTokenizer.h"
#include <include/base/system.h>
#include <include/base/helpers.h>
#include <string.h>

namespace Shaders
{

static EnumMapperA ShaderTokenMapper[] = { 
	{"//!",			TOKEN_METADATA },
	{";",			TOKEN_SEMICOLON	},
	{"=",			TOKEN_EQUALS },
	#undef ITEM
	#define ITEM(name) { MAGIC(name), TOKEN_##name },
	SHADERTOKEN_LIST	
	{"", -1} };


/// Skips whitespace characters
static const char* SkipWhiteSpace(const char* Position)
{
	const char* pos = Position;
	while (*pos == ' ' || *pos == '\t') pos++;
	return pos;
}

/// Skips all characters except those in SkipCharacters and '\0', 
/// returns the first matching position
static const char* SkipUntil(const char* Position, const char* SkipCharacters)
{
	const char* pos = Position;
	for (char c=*pos; c!=0 && !strchr(SkipCharacters, c); c=*++pos);
	return pos;
}


/// Skips all characters in SkipCharacters, returns the first non-matching position
static const char* SkipAll(const char* Position, const char* SkipCharacters)
{
	const char* pos = Position;
	for (char c=*pos; c!=0 && strchr(SkipCharacters, c); c=*++pos);
	return pos;
}

/// Returns next word in quotes
SubString GetNextQuote(const char* Position, int LineNumber)
{
	ASSERT(Position[0] == '\"');

	const char* pos = Position + 1;
	while (true)
	{
		pos = SkipUntil(pos, "\"\\\n\r");
		if (pos[0] == '\\' && pos[1] != 0 && !strchr("\r\n", pos[1]))
		{
			/// Skip escapings
			pos += 2;
		} else break;
	}
	if (*pos != '"')
	{
		ERR(L"Unfinished quotemark in line %d.", LineNumber);
	} else pos++;
	return SubString(Position, pos - Position, TOKEN_STRING);
}

/// Returns next word. Comment lines are one word. Stops before line endings.
SubString GetNextWord(const char* Position, int LineNumber)
{
	const char* begin = Position;

	/// Handle quoted strings
	if (*begin == '"') return GetNextQuote(begin, LineNumber);

	/// Handle special comment tag: "//!" is a valid word for us
	if (begin[0]=='/' && begin[1]=='/' && begin[2]=='!') return SubString(begin, 3, TOKEN_METADATA);

	const char* pos = SkipUntil(begin, "/;= \t\n\r\"");

	if (pos == Position)
	{
		if (pos[0] == '/' && pos[1] == '/')
		{
			/// Comment here, return rest of the line
			pos += 2;
			for (char c=*pos; c!=0 && !strchr("\n\r", c); c=*++pos);
			return SubString(begin, pos - begin, TOKEN_COMMENT_LINE);
		}

		/// Some characters are a word on their own
		if (strchr("/;=", pos[0])) return SubString(pos, 1);
	}

	return SubString(begin, pos - begin);
}


/// Splits the while source code into words
vector<SourceLine*>* Shaders::SplitToWords( const char* Source )
{
	int lineNumber = 1;
	const char* pos = Source;
	vector<SourceLine*>* lines = new vector<SourceLine*>();
	SourceLine* line = NULL;
	while (*pos != 0)
	{
		pos = SkipWhiteSpace(pos);

		/// Skip possible line endings, increment line number
		const char* nextLine = SkipAll(pos, "\r\n");
		if (nextLine != pos)
		{
			if (line) 
			{
				lines->push_back(line);
				line = NULL;
			}
			for (; pos != nextLine; pos++)
			{
				if (*pos == '\r') lineNumber++;
			}
			continue;
		}

		if (line == NULL) line = new SourceLine(lineNumber);

		/// Process next work
		SubString subString = GetNextWord(pos, lineNumber);
		line->SubStrings.push_back(subString);
		pos += subString.Length;
	}
	if (line) lines->push_back(line);
	return lines;
}


SubString::SubString( const char* _Begin, UINT _Length )
	: Begin(_Begin)
	, Length(_Length)
{
	int token = EnumMapperA::GetEnumFromString(ShaderTokenMapper, Begin, Length);
	Token = (token == -1) ? TOKEN_UNKNOWN : (ShaderTokenEnum)token;
}

SubString::SubString( const char* _Begin, UINT _Length, ShaderTokenEnum _Token )
	: Begin(_Begin)
	, Length(_Length)
	, Token(_Token)
{}

string SubString::ToString()
{
	if (Token == TOKEN_STRING) 
	{
		return string(Begin + 1, Length - 2);
	}
	return string(Begin, Length);
}

OWNERSHIP string* SubString::ToStringPtr()
{
	return new string(Begin, Length);
}

SourceLine::SourceLine( int _LineNumber )
	: LineNumber(_LineNumber)
{}

}