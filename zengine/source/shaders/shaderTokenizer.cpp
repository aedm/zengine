#include "shaderTokenizer.h"
#include <include/base/system.h>
#include <include/base/helpers.h>

namespace Shaders
{

  static EnumMapA<ShaderTokenEnum> ShaderTokenMapper = {
    {"//!",	ShaderTokenEnum::TOKEN_METADATA },
    {";",	ShaderTokenEnum::TOKEN_SEMICOLON },
    {":",	ShaderTokenEnum::TOKEN_COLON },
    {"=",	ShaderTokenEnum::TOKEN_EQUALS },
    #undef ITEM
    #define ITEM(name) { MAGIC(name), ShaderTokenEnum::TOKEN_##name },
    SHADERTOKEN_LIST
  };


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
    // ReSharper disable once CppPossiblyErroneousEmptyStatements
    for (char c = *pos; c != 0 && !strchr(SkipCharacters, c); c = *++pos);
    return pos;
  }


  /// Skips all characters in SkipCharacters, returns the first non-matching position
  static const char* SkipAll(const char* Position, const char* SkipCharacters)
  {
    const char* pos = Position;
    // ReSharper disable once CppPossiblyErroneousEmptyStatements
    for (char c = *pos; c != 0 && strchr(SkipCharacters, c); c = *++pos);
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
        /// Skip escaping
        pos += 2;
      }
      else break;
    }
    if (*pos != '"')
    {
      ERR(L"Unfinished quotemark in line %d.", LineNumber);
    }
    else pos++;
    return SubString(Position, UINT(pos - Position), ShaderTokenEnum::TOKEN_STRING);
  }

  /// Returns next word. Comment lines are one word. Stops before line endings.
  SubString GetNextWord(const char* Position, int LineNumber)
  {
    const char* begin = Position;

    /// Handle quoted strings
    if (*begin == '"') return GetNextQuote(begin, LineNumber);

    if (begin[0] == ':') {
      return SubString(begin, 1, ShaderTokenEnum::TOKEN_COLON);
    }

    /// Handle special comment tag: "//!" is a valid word for us
    if (begin[0] == '/' && begin[1] == '/' && begin[2] == '!') {
      return SubString(begin, 3, ShaderTokenEnum::TOKEN_METADATA);
    }

    const char* pos = SkipUntil(begin, "/;= \t\n\r\"");

    if (pos == Position)
    {
      if (pos[0] == '/' && pos[1] == '/')
      {
        /// Comment here, return rest of the line
        pos += 2;
        // ReSharper disable once CppPossiblyErroneousEmptyStatements
        for (char c = *pos; c != 0 && !strchr("\n\r", c); c = *++pos);
        return SubString(begin, UINT(pos - begin), ShaderTokenEnum::TOKEN_COMMENT_LINE);
      }

      /// Some characters are a word on their own
      if (strchr("/;=", pos[0])) return SubString(pos, 1);
    }

    return SubString(begin, UINT(pos - begin));
  }


  /// Splits the while source code into words
  std::vector<SourceLine*>* Shaders::SplitToWords(const char* source)
  {
    int lineNumber = 1;
    const char* pos = source;
    std::vector<SourceLine*>* lines = new std::vector<SourceLine*>();
    SourceLine* line = nullptr;
    while (*pos != 0)
    {
      const char* beforeWhitespaces = pos; /// Position before skipping whitespaces
      pos = SkipWhiteSpace(pos);

      /// Skip possible line endings, increment line number
      const char* beforeLineEndings = pos; /// Position before line endings
      const char* nextLine = SkipAll(pos, "\r\n");
      if (nextLine != pos)
      {
        if (line)
        {
          line->EntireLine.Length = UINT(beforeLineEndings - line->EntireLine.Begin);
          lines->push_back(line);
          line = nullptr;
        }
        for (; pos != nextLine; pos++)
        {
          if (*pos == '\r') lineNumber++;
        }
        continue;
      }

      if (line == nullptr) line = new SourceLine(lineNumber, beforeWhitespaces);

      /// Process next work
      SubString subString = GetNextWord(pos, lineNumber);
      line->SubStrings.push_back(subString);
      pos += subString.Length;
    }
    if (line) lines->push_back(line);
    return lines;
  }


  SubString::SubString(const char* _Begin, UINT _Length)
    : Begin(_Begin)
    , Length(_Length)
  {
    ShaderTokenEnum token = ShaderTokenMapper.GetEnumA(Begin, Length);
    Token = (signed(token) < 0) ? ShaderTokenEnum::TOKEN_UNKNOWN : token;
  }

  SubString::SubString(const char* _Begin, UINT _Length, ShaderTokenEnum _Token)
    : Token(_Token)
    , Begin(_Begin)
    , Length(_Length)
  {}

  std::string SubString::ToString() const
  {
    if (Token == ShaderTokenEnum::TOKEN_STRING)
    {
      return std::string(Begin + 1, Length - 2);
    }
    return std::string(Begin, Length);
  }

  SourceLine::SourceLine(int _LineNumber, const char* LineBegin)
    : LineNumber(_LineNumber)
    , EntireLine(LineBegin, 0, ShaderTokenEnum::TOKEN_UNKNOWN)
  {}

}