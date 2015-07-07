#pragma once

#include "../dom/node.h"
#include <vector>
#include <map>

using namespace std;
class ShaderSource2;

/// Macrolist for global uniforms (name, type, variable/token)
#define GLOBALUSAGE_LIST \
  ITEM(USAGE_TIME,					          FLOAT,			Time)					          \
  ITEM(USAGE_MATRIX_VIEW,				      MATRIX44,		View)					          \
  ITEM(USAGE_MATRIX_PROJECTION,		    MATRIX44,		Projection)				      \
  ITEM(USAGE_MATRIX_TRANSFORMATION,	  MATRIX44,		Transformation)			    \
  ITEM(USAGE_NOISEMAP_SIZE,			      VEC2,			  NoiseMapSize)			      \
  ITEM(USAGE_NOISEMAP_SIZE_RECIP,		  VEC2,			  NoiseMapSizeRecip)		  \
  ITEM(USAGE_RENDERTARGET_SIZE,		    VEC2,			  RenderTargetSize)		    \
  ITEM(USAGE_RENDERTARGET_SIZE_RECIP,	VEC2,			  RenderTargetSizeRecip)	\
  ITEM(USAGE_VIEWPORT_SIZE,			      VEC2,			  ViewportSize)			      \
  ITEM(USAGE_PIXEL_SIZE,				      VEC2,			  PixelSize)				      \
  ITEM(USAGE_DIFFUSE_COLOR,			      VEC4,			  DiffuseColor)			      \
  ITEM(USAGE_AMBIENT_COLOR,			      VEC4,			  AmbientColor)			      \
  ITEM(USAGE_DEPTH_BIAS,				      FLOAT,			DepthBias)				      \

enum class ShaderGlobalType {
#undef ITEM
#define ITEM(name, type, variable) name,
  GLOBALUSAGE_LIST

  LOCAL,	/// For non-global uniforms
};

extern const EnumMapperA GlobalUniformMapper[];
extern const NodeType GlobalUniformTypes[];
extern const int GlobalUniformOffsets[];

struct Globals {
#undef ITEM
#define ITEM(name, type, token) type##_TYPE token;
  GLOBALUSAGE_LIST
};

/// Shader parameter, becomes a slot
/// ":param vec4 MyColor" or ":param sampler2d MyTexture"
struct ShaderStubParameter {
  NodeType type;
  string name;
};

/// Shader variables are outputs of a shader stage and inputs of the next shader stage.
/// ":output vec4 MyColor" -- creates "outMyColor" output variable.
/// ":input vec4 MyColor" -- creates "inMyColor" input variable.
struct ShaderStubVariable {
  NodeType type;
  string name;
};

/// --
struct ShaderStubGlobal {
  NodeType type;
  string name;
  ShaderGlobalType usage;
};


/// All metadata collected from a shader stub source.
struct ShaderStubMetadata {
  ShaderStubMetadata(const string& name, NodeType returnType,
                     const string& strippedSource,
                     const vector<OWNERSHIP ShaderStubParameter*>& parameters,
                     const vector<ShaderStubGlobal*>& globals,
                     const vector<ShaderStubVariable*>& inputs,
                     const vector<ShaderStubVariable*>& outputs);

  ~ShaderStubMetadata();

  const string name;
  const NodeType returnType;

  const string strippedSource;

  const vector<ShaderStubParameter*> parameters;
  const vector<ShaderStubGlobal*> globals;
  const vector<ShaderStubVariable*>	inputs;
  const vector<ShaderStubVariable*>	outputs;
};


class ShaderStub: public Node {
public:
  ShaderStub();
  ShaderStub(const ShaderStub& original);
  virtual ~ShaderStub();

  virtual Node* Clone() const;

  void SetStubSource(const string& source);
  const string& GetStubSource() const;
  ShaderStubMetadata* GetStubMetadata() const;

  ShaderSource2* GetShaderSource();

  /// Get slot by shader parameter name
  Slot* GetSlotByParameter(ShaderStubParameter*);
  Slot* GetSlotByParameterName(const string& name);

protected:
  /// Handle received messages
  virtual void HandleMessage(Slot* slot, NodeMessage message,
                             const void* payload) override;

  /// Metadata
  ShaderStubMetadata* mMetadata;

  string mSource;

  ShaderSource2* mShaderSrc;

  /// Maps stub parameters to stub slots
  map<ShaderStubParameter*, Slot*> mParameterSlotMap;
  map<string, Slot*> mParameterNameSlotMap;
};

