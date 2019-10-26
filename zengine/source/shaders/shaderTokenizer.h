#pragma once


#include <include/base/defines.h>
#include <vector>
#include <string>

namespace Shaders {

  /// Macrolist for shader tokens
#define SHADERTOKEN_LIST	\
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
    SHADERTOKEN_LIST
  };

  struct SubString {
    SubString(const char* Begin, UINT Length);
    SubString(const char* Begin, UINT Length, ShaderTokenEnum Token);

    ShaderTokenEnum Token;
    const char* Begin;
    UINT Length;
    std::string ToString() const;
  };

  struct SourceLine {
    int					LineNumber;
    std::vector<SubString>	SubStrings;
    SubString			EntireLine;

    SourceLine(int LineNumber, const char* LineBegin);
  };

  OWNERSHIP std::vector<SourceLine*>* SplitToWords(const char* Text);
}