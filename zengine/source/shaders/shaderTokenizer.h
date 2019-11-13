#pragma once


#include <include/base/defines.h>
#include <vector>
#include <string>

namespace Shaders {

  /// Macro list for shader tokens
#define SHADER_TOKEN_LIST	\
	ITEM(float)				      \
	ITEM(vec2)				      \
	ITEM(vec3)				      \
	ITEM(vec4)				      \
	ITEM(mat4)				      \
	ITEM(sampler2D)			    \
	ITEM(sampler2DMS)  	    \
	ITEM(sampler2DShadow)   \
	ITEM(image2D)           \
	ITEM(buffer)            \
	ITEM(void)				      \
	ITEM(name)				      \
	ITEM(returns)			      \
	ITEM(section)			      \
	ITEM(min)				        \
	ITEM(max)				        \
	ITEM(def)				        \
	ITEM(scale)				      \
	ITEM(linear)			      \
	ITEM(logarithmic)	      \
	ITEM(uniform)			      \
	ITEM(input)				      \
	ITEM(output)			      \
	ITEM(param)				      \
	ITEM(global)			      \
	ITEM(using)				      \
	ITEM(auto)				      \
	ITEM(option)			      \
	ITEM(ui)				        \
	ITEM(macro)				      \
	ITEM(choice)			      \

  enum class ShaderTokenEnum {
    TOKEN_UNKNOWN,
    TOKEN_COMMENT_LINE,
    TOKEN_METADATA,				// "//!"
    TOKEN_SEMICOLON,			// ";"
    TOKEN_COLON,				  // ":"
    TOKEN_EQUALS,				  // "="
    TOKEN_STRING,
#undef ITEM
#define ITEM(name) TOKEN_##name,
    SHADER_TOKEN_LIST
  };

  struct SubString {
    SubString(const char* begin, UINT length);
    SubString(const char* begin, UINT length, ShaderTokenEnum token);

    ShaderTokenEnum mToken = ShaderTokenEnum::TOKEN_UNKNOWN;
    const char* mBegin = nullptr;
    UINT mLength = 0;

    std::string ToString() const;
  };

  struct SourceLine {
    int mLineNumber = -1;
    std::vector<SubString> mSubStrings;
    SubString mEntireLine;

    SourceLine(int lineNumber, const char* lineBegin);
  };

  OWNERSHIP std::vector<SourceLine*>* SplitToWords(const char* source);
}