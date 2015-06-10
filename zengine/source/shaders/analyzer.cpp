#include "analyzer.h"
#include <include/shaders/shaders.h>
#include <boost/foreach.hpp>
#include <string>

const EnumMapperA GlobalUniformMapper[] = {
#undef ITEM
#define ITEM(name, type, token) { "g" MAGIC(token), GlobalUniform::name },
	GLOBALUSAGE_LIST
	{ "", -1 }
};

namespace Shaders
{

void Analysis::AnalyzedLocal::Reset( SharedString _UniformName, NodeType _Type )
{
	UniformName =_UniformName;
	Type = _Type;
	UISectionName = NULL;
	Active = true;
}

LocalDesc* Analysis::AnalyzedLocal::ToLocalDesc()
{
	ASSERT(Active);
	LocalDesc* ret = new LocalDesc(Type, UniformName, UISectionName, Floats);
	Active = false;
	Floats = NULL;
	return ret;
}

void Analysis::AnalyzedLocalFloat::Reset(SharedString UIName)
{
	this->UIName = UIName;
	UIMinimum = 0;
	UIMaximum = 5;
	DefaultValue = 0;
	Scale = LocalFloatDesc::SCALE_LINEAR;
	Active = true;
}

LocalFloatDesc Analysis::AnalyzedLocalFloat::ToLocalFloatDesc()
{
	Active = false;
	return LocalFloatDesc(UIName, UIMinimum, UIMaximum, DefaultValue, Scale);
}

OWNERSHIP ShaderMetadata* Analysis::FromText(const char* _FileName, OWNERSHIP const char* Source )
{
	FileName = _FileName;
	State = SOURCE_STATE_VOID;

	vector<SourceLine*>* lines = SplitToWords(Source);

	for (SourceLine* line : *lines)
	{
		CurrentLineNumber = line->LineNumber;
		if (line->SubStrings[0].Token == TOKEN_METADATA)
		{
			AnalyzeMetadataLine(line);
		}
		else if (line->SubStrings[0].Token != TOKEN_COMMENT_LINE)
		{
			Finish();
			if (line->SubStrings[0].Token == TOKEN_uniform)
			{
				AnalyzeUniformLine(line);
			} 
		}
		delete line;
	}
	Finish();
	delete lines;

	return new ShaderMetadata(Options, Choices, Locals);
}

void Analysis::AnalyzeUniformLine( SourceLine* line ) 
{
	if (line->SubStrings.size() == 4 && line->SubStrings[3].Token == TOKEN_SEMICOLON) 
	{
		NodeType type = GetOpTypeFromToken(line->SubStrings[1].Token);
		if (int(type) >= 0)
		{
			SubString& name = line->SubStrings[2];
			if (*name.Begin == 'g')
			{
				// Global uniform
				if (EnumMapperA::GetEnumFromString(GlobalUniformMapper, name.Begin, name.Length) < 0)
				{
					ERR("%s:%d Unrecognized global uniform '%s'.", FileName, 
						CurrentLineNumber, name.ToString().c_str());
				}
			} else {
				// Local uniform
				CurrentLocal.Reset(make_shared<string>(name.ToString()), type);
				State = SOURCE_STATE_LOCAL;
			}
			return;
		} else {
			ERR("%s:%d Unrecognized uniform type '%s'.", FileName, 
				CurrentLineNumber, line->SubStrings[1].ToString().c_str());
		}
	}
	ERR("%s:%d Uniforms should be defined like \"uniform [type] [name];\".", FileName, 
		CurrentLineNumber);
}

NodeType Analysis::GetOpTypeFromToken( ShaderTokenEnum Token )
{
	switch (Token)
	{
	case TOKEN_float:		return NodeType::FLOAT;
	case TOKEN_vec2:		return NodeType::VEC2;
	case TOKEN_vec3:		return NodeType::VEC3;
	case TOKEN_vec4:		return NodeType::VEC4;
	case TOKEN_mat4:		return NodeType::MATRIX44;
	case TOKEN_sampler2D:	return NodeType::TEXTURE;
	default: return (NodeType)-1;
	}
}

void Analysis::AnalyzeMetadataLine( SourceLine* line )
{
	switch (State)
	{
	case SOURCE_STATE_VOID:
		WARN("%s:%d Metadata for nothing", FileName, CurrentLineNumber);
		break;
	case SOURCE_STATE_LOCAL:
		AnalyzeMetadataLine_Local(line);
		break;
	}
}

void Analysis::AnalyzeMetadataLine_Local( SourceLine* line )
{
	UINT pos = 1;
	for (; pos+2 < line->SubStrings.size(); pos += 3)
	{
		if (line->SubStrings[pos+1].Token != TOKEN_EQUALS)
		{
			ERR("%s:%d Metadata syntax should be \"[attribute] = [value]\".", FileName, 
				CurrentLineNumber);
			return;
		}

		CurrentAttribute = &line->SubStrings[pos];
		SubString val = line->SubStrings[pos+2];
		switch (CurrentAttribute->Token)
		{
		case TOKEN_section:
			if (CurrentLocal.UISectionName) 
			{
				WARN("%s:%d Section name for uniform '%s' already defined.", FileName, 
					line->LineNumber, CurrentLocal.UniformName);
				delete CurrentLocal.UISectionName;
			}
			CurrentLocal.UISectionName = val.ToStringPtr();
			break;
		case TOKEN_ui:
			FinishLocalFloat();
			CurrentLocalFloat.Reset(make_shared<string>(val.ToString()));
			break;
		case TOKEN_min:
			if (IsCurrentLocalFloatActive()) SubStringToFloat(val, &CurrentLocalFloat.UIMinimum);
			break;
		case TOKEN_max:
			if (IsCurrentLocalFloatActive()) SubStringToFloat(val, &CurrentLocalFloat.UIMaximum);
			break;
		case TOKEN_def:
			if (IsCurrentLocalFloatActive()) SubStringToFloat(val, &CurrentLocalFloat.DefaultValue);
			break;
		case TOKEN_scale:
			if (IsCurrentLocalFloatActive())
			{
				switch (val.Token)
				{
				case TOKEN_linear:		
					CurrentLocalFloat.Scale = LocalFloatDesc::SCALE_LINEAR;
					break;
				case TOKEN_logarithmic:		
					CurrentLocalFloat.Scale = LocalFloatDesc::SCALE_LOGARITHMIC;
					break;
				default:
					ERR("%s:%d Parameter '%s' is not a valid scale. Use 'linear' or 'logarithmic'.", 
						FileName, CurrentLineNumber, val.ToString().c_str());
				}
			}
			break;
		default:
			ERR("%s:%d Unknown metadata attribute '%s'.", FileName, CurrentLineNumber, 
				CurrentAttribute->ToString().c_str());
			break;
		}
	}

	if (pos < line->SubStrings.size())
	{
		ERR("%s:%d Metadata '%s' not understood. Use 'attribute=value' for defining metadata.", 
			FileName, CurrentLineNumber, line->SubStrings[pos].ToString().c_str());
	}
}

void Analysis::Finish()
{
	switch (State)
	{
	case SOURCE_STATE_VOID: break;
	case SOURCE_STATE_LOCAL:
		FinishLocal();
		break;
	}
	State = SOURCE_STATE_VOID;
}

void Analysis::FinishLocal()
{
	if (CurrentLocal.Active)
	{
		FinishLocalFloat();
		Locals.push_back(CurrentLocal.ToLocalDesc());
	}
}

void Analysis::FinishLocalFloat()
{
	if (CurrentLocalFloat.Active) 
	{
		if (CurrentLocal.Floats == NULL) 
		{
			CurrentLocal.Floats = new vector<LocalFloatDesc>();
		}
		CurrentLocal.Floats->push_back(CurrentLocalFloat.ToLocalFloatDesc());
	}
}

bool Analysis::IsCurrentLocalFloatActive()
{
	if (CurrentLocalFloat.Active) return true;
	ERR("%s:%d Define 'ui' parameter for uniform '%s' before defining '%s'.", FileName, 
		CurrentLineNumber, CurrentLocal.UniformName->c_str(), CurrentAttribute->ToString().c_str());
	return false;
}

void Analysis::SubStringToFloat( SubString& Value, float* oTarget )
{
	try 
	{
		*oTarget = std::stof(Value.ToString(), NULL);
	}
	catch (const std::invalid_argument&) 
	{
		ERR("%s:%d Can't parse value of '%s' parameter '%s'.", FileName, 
			CurrentLineNumber, CurrentAttribute->ToString().c_str(), Value.ToString());
	}
	catch (const std::out_of_range&) 
	{
		ERR("%s:%d Value of '%s' parameter '%s' is out of range.", FileName, 
			CurrentLineNumber, CurrentAttribute->ToString().c_str(), Value.ToString());
	}
}

} // namespace Shaders