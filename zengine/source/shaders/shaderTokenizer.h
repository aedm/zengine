#pragma once


#include <include/base/defines.h>
#include <vector>
//#include <string>
#include <string_view>

using std::string_view;
using std::vector;

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
    const ShaderTokenEnum mToken = ShaderTokenEnum::TOKEN_UNKNOWN;
    const string_view mStringView;

    SubString(const string_view& stringView, ShaderTokenEnum token);
    static SubString FromString(const char* begin, size_t length);
  };

  struct SourceLine {
    const int mLineNumber;
    const string_view mEntireLine;
    const vector<SubString> mSubStrings;
    SourceLine(int lineNumber, const string_view& entireLine, vector<SubString>&& subStrings);
  };

  OWNERSHIP vector<SourceLine> SplitToWords(const char* source);
}