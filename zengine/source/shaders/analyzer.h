#pragma once

#include "shaderTokenizer.h"
#include <include/shaders/shaderMetadata.h>
#include <include/base/helpers.h>

namespace Shaders
{

class Analysis
{
public:
	OWNERSHIP ShaderMetadata*	FromText(const char* FileName, const char* Source);

private:
	enum SourceAnalysisState
	{
		SOURCE_STATE_VOID,
		SOURCE_STATE_LOCAL
	};

	/// See LocalFloatDesc
	struct AnalyzedLocalFloat
	{
		void					Reset(SharedString UIName);
		LocalFloatDesc			ToLocalFloatDesc();
		SharedString			UIName;
		float					UIMinimum;
		float					UIMaximum;
		float					DefaultValue;
		LocalFloatDesc::ScaleType Scale;
		bool					Active;
		AnalyzedLocalFloat(): Active(false) {}
	};

	/// See LocalDesc
	struct AnalyzedLocal 
	{
		void					Reset(SharedString UniformName, NodeTypeEnum Type);
		LocalDesc*				ToLocalDesc();
		NodeTypeEnum				Type;
		SharedString			UniformName;
		string*					UISectionName;
		vector<LocalFloatDesc>*	Floats;
		bool					Active;
		AnalyzedLocal(): Active(false), Floats(NULL) {}
	};

	void						AnalyzeUniformLine(SourceLine* line);
	void						AnalyzeMetadataLine(SourceLine* line);
	void						AnalyzeMetadataLine_Local(SourceLine* line);

	static NodeTypeEnum			GetOpTypeFromToken(ShaderTokenEnum Token);
	bool						IsCurrentLocalFloatActive();
	void						SubStringToFloat(SubString& Value, float* oTarget);

	/// Finishes current section
	void						Finish();
	void						FinishLocal();
	void						FinishLocalFloat();

	/// For logging
	const char*					FileName;
	int							CurrentLineNumber;
	SubString*					CurrentAttribute;

	SourceAnalysisState			State;
	AnalyzedLocal				CurrentLocal; 
	AnalyzedLocalFloat			CurrentLocalFloat; 

	/// Builders
	vector<LocalDesc*>			Locals;	
	vector<ShaderOption*>		Options;	
	vector<ShaderChoice*>		Choices;	
};


} // namespace Shaders