#include "shaderTokenizer.h"
#include <include/base/system.h>
#include <include/base/helpers.h>

namespace Shaders {
  static EnumMapA<ShaderTokenEnum> ShaderTokenMapper = {
    {"//!", ShaderTokenEnum::TOKEN_METADATA},
    {";", ShaderTokenEnum::TOKEN_SEMICOLON},
    {":", ShaderTokenEnum::TOKEN_COLON},
    {"=", ShaderTokenEnum::TOKEN_EQUALS},
#undef ITEM
#define ITEM(name) { MAGIC(name), ShaderTokenEnum::TOKEN_##name },
    SHADER_TOKEN_LIST
  };


  /// Skips whitespace characters
  static const char* SkipWhiteSpace(const char* position) {
    const char* pos = position;
    while (*pos == ' ' || *pos == '\t') pos++;
    return pos;
  }

  /// Skips all characters except those in SkipCharacters and '\0', 
  /// returns the first matching position
  static const char* SkipUntil(const char* position, const char* skipCharacters) {
    const char* pos = position;
    // ReSharper disable once CppPossiblyErroneousEmptyStatements
    for (char c = *pos; c != 0 && !strchr(skipCharacters, c); c = *++pos);
    return pos;
  }


  /// Skips all characters in SkipCharacters, returns the first non-matching position
  static const char* SkipAll(const char* position, const char* skipCharacters) {
    const char* pos = position;
    // ReSharper disable once CppPossiblyErroneousEmptyStatements
    for (char c = *pos; c != 0 && strchr(skipCharacters, c); c = *++pos);
    return pos;
  }

  /// Returns next word in quotes
  SubString GetNextQuote(const char* position, int lineNumber) {
    ASSERT(position[0] == '\"');

    const char* pos = position + 1;
    while (true) {
      pos = SkipUntil(pos, "\"\\\n\r");
      if (pos[0] == '\\' && pos[1] != 0 && !strchr("\r\n", pos[1])) {
        /// Skip escaping
        pos += 2;
      }
      else break;
    }
    if (*pos != '"') {
      ERR(L"Unfinished quotemark in line %d.", lineNumber);
    }
    else pos++;
    return SubString(string_view(position, pos - position), 
      ShaderTokenEnum::TOKEN_STRING);
  }

  /// Returns next word. Comment lines are one word. Stops before line endings.
  SubString GetNextWord(const char* position, int lineNumber) {
    const char* begin = position;

    /// Handle quoted strings
    if (*begin == '"') return GetNextQuote(begin, lineNumber);

    if (begin[0] == ':') {
      return SubString(string_view(begin, 1), ShaderTokenEnum::TOKEN_COLON);
    }

    /// Handle special comment tag: "//!" is a valid word for us
    if (begin[0] == '/' && begin[1] == '/' && begin[2] == '!') {
      return SubString(string_view(begin, 3), ShaderTokenEnum::TOKEN_METADATA);
    }

    const char* pos = SkipUntil(begin, "/;= \t\n\r\"");

    if (pos == position) {
      if (pos[0] == '/' && pos[1] == '/') {
        /// Comment here, return rest of the line
        pos += 2;
        // ReSharper disable once CppPossiblyErroneousEmptyStatements
        for (char c = *pos; c != 0 && !strchr("\n\r", c); c = *++pos);
        return SubString(string_view(begin, pos - begin), 
          ShaderTokenEnum::TOKEN_COMMENT_LINE);
      }

      /// Some characters are a word on their own
      if (strchr("/;=", pos[0])) return SubString::FromString(pos, 1);
    }

    return SubString::FromString(begin, pos - begin);
  }


  /// Splits the while source code into words
  std::vector<SourceLine> SplitToWords(const char* source) {
    int lineNumber = 1;
    const char* pos = source;
    std::vector<SourceLine> lines;
    //SourceLine* line = nullptr;

    const char* lineStart = nullptr;
    vector<SubString> subStrings;

    while (*pos != 0) {
      const char* beforeWhitespaces = pos; /// Position before skipping whitespaces
      pos = SkipWhiteSpace(pos);

      /// Skip possible line endings, increment line number
      const char* beforeLineEndings = pos; /// Position before line endings
      const char* nextLine = SkipAll(pos, "\r\n");
      if (nextLine != pos) {
        if (lineStart) {
          const size_t lineLength = beforeLineEndings - lineStart;
          lines.emplace_back(
            lineNumber, string_view(lineStart, lineLength), std::move(subStrings));
          subStrings.clear();
          lineStart = nullptr;
        }
        for (; pos != nextLine; pos++) {
          if (*pos == '\r') lineNumber++;
        }
        continue;
      }

      if (lineStart == nullptr) {
        lineStart = beforeWhitespaces;
      }

      /// Process next word
      SubString subString = GetNextWord(pos, lineNumber);
      subStrings.push_back(subString);
      pos += subString.mStringView.size();
    }

    /// Add last line
    if (lineStart) {
      lines.emplace_back(
        lineNumber, string_view(lineStart, pos - lineStart), std::move(subStrings));
    }
    return lines;
  }


  SubString SubString::FromString(const char* begin, size_t length)
  //: mStringView(begin, length)
  //: mBegin(begin)
  //, mLength(length)
  {
    string_view stringView(begin, length);
    ShaderTokenEnum token = ShaderTokenMapper.GetEnumA(stringView);
    ShaderTokenEnum finalToken =
      (signed(token) < 0) ? ShaderTokenEnum::TOKEN_UNKNOWN : token;
    return SubString(stringView, finalToken);
  }

  SubString::SubString(const string_view& stringView, ShaderTokenEnum token)
    : mToken(token)
      , mStringView(stringView) {}

  //std::string SubString::ToString() const
  //{
  //  if (mToken == ShaderTokenEnum::TOKEN_STRING) {
  //    return std::string(mBegin + 1, mLength - 2);
  //  }
  //  return std::string(mBegin, mLength);
  //}

  SourceLine::SourceLine(int lineNumber, const string_view& entireLine,
                         vector<SubString>&& subStrings)
    : mLineNumber(lineNumber)
      , mEntireLine(entireLine)
      , mSubStrings(subStrings) {}

}
