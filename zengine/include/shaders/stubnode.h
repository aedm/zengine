#pragma once

#include "../dom/node.h"
#include "../nodes/valuenodes.h"
#include <vector>
#include <map>

using namespace std;

/// Macro list for global uniforms (name, type, variable/token)
#define GLOBALUSAGE_LIST \
  ITEM(USAGE_TIME,					                      FLOAT,			  Time)					            \
  ITEM(USAGE_MATRIX_VIEW,				                  MATRIX44,		  View)					            \
  ITEM(USAGE_MATRIX_PROJECTION,		                MATRIX44,		  Projection)				        \
  ITEM(USAGE_MATRIX_TRANSFORMATION,	              MATRIX44,		  Transformation)			      \
  ITEM(USAGE_NOISEMAP_SIZE,			                  VEC2,			    NoiseMapSize)			        \
  ITEM(USAGE_NOISEMAP_SIZE_RECIP,		              VEC2,			    NoiseMapSizeRecip)		    \
  ITEM(USAGE_RENDERTARGET_SIZE,		                VEC2,			    RenderTargetSize)		      \
  ITEM(USAGE_RENDERTARGET_SIZE_RECIP,	            VEC2,			    RenderTargetSizeRecip)	  \
  ITEM(USAGE_VIEWPORT_SIZE,			                  VEC2,			    ViewportSize)			        \
  ITEM(USAGE_PIXEL_SIZE,				                  VEC2,			    PixelSize)				        \
  ITEM(USAGE_DIFFUSE_COLOR,			                  VEC4,			    DiffuseColor)			        \
  ITEM(USAGE_AMBIENT_COLOR,			                  VEC4,			    AmbientColor)			        \
  ITEM(USAGE_DEPTH_BIAS,				                  FLOAT,			  DepthBias)				        \
  ITEM(USAGE_DEPTH_BUFFER,				                TEXTURE,		  DepthBufferSource)		    \
  ITEM(USAGE_GBUFFER_SOURCE_A,                    TEXTURE,		  GBufferSourceA)	          \
  ITEM(USAGE_GBUFFER_SAMPLE_COUNT,		            FLOAT,        GBufferSampleCount)       \
  ITEM(USAGE_SECONDARY_TEXTURE,                   TEXTURE,		  SecondaryTexture)         \
  ITEM(USAGE_SKYLIGHT_TEXTURE,                    TEXTURE,		  SkylightTexture)          \
  ITEM(USAGE_SKYLIGHT_COLOR_TEXTURE,              TEXTURE,		  SkylightColorTexture)     \
  ITEM(USAGE_SKYLIGHT_PROJECTION,                 MATRIX44,		  SkylightMatrix)           \
  ITEM(USAGE_POSTPROCESS_GAUSSTEX,                TEXTURE,      PPGauss)		              \
  ITEM(USAGE_POSTPROCESS_GAUSSTEX_PIXEL_SIZE,     VEC2,         PPGaussPixelSize)		      \
  ITEM(USAGE_POSTPROCESS_GAUSSTEX_RELATIVE_SIZE,  VEC2,         PPGaussRelativeSize)		  \


enum class ShaderGlobalType {
#undef ITEM
#define ITEM(name, type, variable) name,
	GLOBALUSAGE_LIST

	LOCAL,	/// For non-global uniforms
};

extern const EnumMapperA GlobalUniformMapper[];
extern const NodeType GlobalUniformTypes[];
extern const int GlobalUniformOffsets[];

/// A struct to store global uniforms
struct Globals {
#undef ITEM
#define ITEM(name, type, token) NodeTypes<NodeType::type>::Type token;
	GLOBALUSAGE_LIST
};

/// Stub parameter, becomes a slot
/// ":param vec4 MyColor" or ":param sampler2d MyTexture"
struct StubParameter {
	NodeType mType;
	SharedString mName;
};

/// Stub variables are outputs of a shader stage and inputs of the next shader stage.
/// ":output vec4 MyColor" -- creates "outMyColor" output variable.
/// ":input vec4 MyColor" -- creates "inMyColor" input variable.
struct StubVariable {
	NodeType type;
	string name;
};

/// --
struct StubGlobal {
	NodeType type;
	string name;
	ShaderGlobalType usage;
  bool isMultiSampler; // type is "sampler2DMS"
  bool isShadow; // type is "sampler2DShadow"
};


/// All metadata collected from a stub source.
struct StubMetadata {
	StubMetadata(const string& name, NodeType returnType,
				 const string& strippedSource,
				 const vector<OWNERSHIP StubParameter*>& parameters,
				 const vector<StubGlobal*>& globals,
				 const vector<StubVariable*>& inputs,
				 const vector<StubVariable*>& outputs);

	~StubMetadata();

	/// Value of the ":name" directive.
	const string name;

	/// Value of the ":returns" directive.
	const NodeType returnType;

	/// The source without any directives.
	const string strippedSource;

	/// Parameters of the stub. These become slots. (":param")
	const vector<StubParameter*> parameters;

	/// List of globals. (":global")
	const vector<StubGlobal*> globals;

	/// List of stage inputs. (":input")
	const vector<StubVariable*>	inputs;

	/// List of stage outputs. (":output")
	const vector<StubVariable*>	outputs;
};


/// A stub is a part of a shader. It contains shader source code annotated
/// by directives. Using these annotations, stubs can depend on each other and
/// on other nodes likes textures and floats. 
class StubNode: public Node {
	friend class ShaderBuilder;

public:
	StubNode();
	StubNode(const StubNode& original);
	virtual ~StubNode();

	/// Returns the metadata containing information about the stub source.
	StubMetadata* GetStubMetadata() const;

	/// Get slot by shader parameter name
	Slot* GetSlotByParameter(StubParameter*);
	Slot* GetSlotByParameterName(const string& name);

	/// Source of the stub
	StringSlot mSource;

protected:
	/// Performs metadata analysis on the new stub source.
	void HandleSourceChange();

	/// Handle received messages
	virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;

	/// Metadata
	StubMetadata* mMetadata;

	/// Maps stub parameters to stub slots
	map<StubParameter*, Slot*> mParameterSlotMap;
	map<string, Slot*> mParameterNameSlotMap;
};


typedef TypedSlot<NodeType::SHADER_STUB, StubNode> StubSlot;

