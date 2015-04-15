#pragma once

#include "shaders.h"
#include <string>

using namespace std;

class ShaderSource
{
public:
	~ShaderSource();

	static OWNERSHIP ShaderSource*		FromFile( const wchar_t* VertexShaderFile, 
												  const wchar_t* FragmentShaderFile );
	static OWNERSHIP ShaderSource*		FromSource( OWNERSHIP const char* VertexSource, 
		                                            OWNERSHIP const char* FragmentSource);

	ShaderMetadata*						Metadata;

	OWNERSHIP static Shader*			Build(const shared_ptr<ShaderSource>& Source);

	/// TODO: restrict visibility. Now it's public, because f** C++ doesn't allow 
	/// static functions to access private members
	ShaderSource();

	const char*							VertexSource;
	const char*							FragmentSource;
	ShaderMetadata*						VertexMetadata;
	ShaderMetadata*						FragmentMetadata;

private:
	LocalDesc*							GetLocalDesc(const string& UniformName);
};

/// Helper class for building shaders
class ShaderBuilder
{
public:
	/// Build a shader program from a simple vertex and fragment shader

	/// Extract metadata from a shader source
	static OWNERSHIP ShaderMetadata*	FromText(const char* FileName, const char* Source);

	/// Merge metadata info
	static OWNERSHIP ShaderMetadata*	Merge(const vector<ShaderMetadata*>& MetadataList);

};