#pragma once

#include "../base/types.h"
#include <vector>
#include <map>

using namespace std;

struct LocalFloatDesc;
struct ShaderOption;
struct ShaderChoice;
class LocalDesc;

/// Description of a shader source
struct ShaderMetadata
{
	ShaderMetadata(OWNERSHIP const vector<ShaderOption*>& Options, 
		OWNERSHIP const vector<ShaderChoice*>& Choices,
		OWNERSHIP const vector<LocalDesc*>& Locals);

	~ShaderMetadata();

	/// Possible compile options in the shader source
	const vector<ShaderOption*>			Options;

	/// Possible compile choices in the shader source
	const vector<ShaderChoice*>			Choices;

	/// Possible local uniforms
	const vector<LocalDesc*>			Locals;
};


/// Compile settings for a built shader program: all options and choices 
struct CompileSettings
{
	CompileSettings(const ShaderMetadata* Desc,
		const map<ShaderOption*, bool>& Options, 
		const map<ShaderChoice*, int>& Choices);

	/// Description of the shader source
	const ShaderMetadata*					Desc;

	/// All possible options and choices and their selected values
	const map<ShaderChoice*, int>		Choices;
	const map<ShaderOption*, bool>		Options;
};


/// Shader compile option, like "#define USE_DEPTH_BIAS"
struct ShaderOption
{
	ShaderOption(const string& UIName, const string& Macro);

	/// UI name to display 
	const string				UIName;				

	/// Shader macro to be defined upon activation
	const string				Macro;				
};


/// Shader compile choice, like "#define BLEND_SUBTRACTIVE" 
/// or "#define BLEND_ADDITIVE"
struct ShaderChoice
{
	struct Item
	{
		/// UI radiobutton name to display
		string					UIName;				

		/// Shader macro to be defined if chosen
		string					Macro;				
	};

	ShaderChoice(const vector<Item>& Items);

	const vector<Item>			Items;
};

/// Description of a local uniform in the shader source, like "uniform vec3 Color". 
class LocalDesc
{
public:
	LocalDesc(NodeType UniformType, SharedString UniformName, string* UISectionName,
		OWNERSHIP const vector<LocalFloatDesc>* Elements);
	~LocalDesc();

	const NodeType					UniformType;	/// Type (float, vec2...)
	SharedString					UniformName;	/// Uniform variable name in shader
	const string*					UISectionName;	/// UI section name

	const vector<LocalFloatDesc>*	Elements;
};


/// One float element of the LocalDesc. If a local uniform is a vector like vec4,
/// it may consist of several float variables on the UI.
struct LocalFloatDesc
{
	enum ScaleType
	{
		SCALE_LINEAR,
		SCALE_LOGARITHMIC
	};

	LocalFloatDesc(SharedString UIName, float UIMinimum, 
		float UIMaximum, float DefaultValue, ScaleType Scale);

	/// UI name of component
	SharedString					UIName;			

	/// UI minimum value
	const float						UIMinimum;		

	/// UI maximum value
	const float						UIMaximum;		

	/// Default value
	const float						DefaultValue;	

	/// UI scale type
	const ScaleType					Scale;			
};
