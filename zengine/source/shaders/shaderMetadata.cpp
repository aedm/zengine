#include <include/shaders/shadermetadata.h>
#include <include/base/defines.h>
#include <boost/foreach.hpp>

ShaderOption::ShaderOption( const string& _UIName, const string& _Macro )
	: UIName(_UIName)
	, Macro(_Macro)
{}


ShaderChoice::ShaderChoice( const vector<Item>& _Items )
	: Items(_Items)
{}


ShaderMetadata::ShaderMetadata(	OWNERSHIP const vector<ShaderOption*>& _Options, 
	OWNERSHIP const vector<ShaderChoice*>& _Choices,
	OWNERSHIP const vector<LocalDesc*>& _Locals )
	: Options(_Options)
	, Choices(_Choices)
	, Locals(_Locals)
{}

ShaderMetadata::~ShaderMetadata()
{
	for (ShaderOption* option : Options) delete option;
	for (ShaderChoice* choice : Choices) delete choice;
	for (LocalDesc* local : Locals) delete local;
}

CompileSettings::CompileSettings(const ShaderMetadata* _Desc, 
	const map<ShaderOption*, bool>& _Options, 
	const map<ShaderChoice*, int>& _Choices )
	: Desc(_Desc)
	, Options(_Options)
	, Choices(_Choices)
{}


LocalDesc::LocalDesc( NodeType _UniformType, SharedString _UniformName, 
	string* _UISectionName, const vector<LocalFloatDesc>* _Elements )
	: UniformType(_UniformType)
	, UniformName(_UniformName)
	, UISectionName(_UISectionName)
	, Elements(_Elements)
{}


LocalDesc::~LocalDesc()
{
	SafeDelete(UISectionName);
	SafeDelete(Elements);
}


LocalFloatDesc::LocalFloatDesc( SharedString _UIName, float _UIMinimum, 
	float _UIMaximum, float _DefaultValue, ScaleType _Scale )
	: UIName(_UIName)
	, UIMinimum(_UIMinimum)
	, UIMaximum(_UIMaximum)
	, DefaultValue(_DefaultValue)
	, Scale(_Scale)
{}
