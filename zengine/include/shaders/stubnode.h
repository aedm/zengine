#pragma once

#include "../dom/node.h"
#include "../nodes/valuenodes.h"
#include "shadervaluetype.h"
#include <vector>
#include <map>

using namespace std;

/// Macro list for global uniforms (name/usage, type)
#define GLOBALUNIFORM_LIST \
  ITEM(Time,                          ShaderValueType::FLOAT) \
  ITEM(Camera,                        ShaderValueType::MATRIX44) \
  ITEM(World,                         ShaderValueType::MATRIX44) \
  ITEM(View,                          ShaderValueType::MATRIX44) \
  ITEM(Projection,                    ShaderValueType::MATRIX44) \
  ITEM(Transformation,                ShaderValueType::MATRIX44) \
  ITEM(SkylightCamera,                ShaderValueType::MATRIX44) \
  ITEM(SkylightProjection,            ShaderValueType::MATRIX44) \
  ITEM(SkylightTransformation,        ShaderValueType::MATRIX44) \
  ITEM(SkylightTextureSizeRecip,      ShaderValueType::FLOAT) \
  ITEM(SkylightDirection,             ShaderValueType::VEC3) \
  ITEM(SkylightColor,                 ShaderValueType::VEC3) \
  ITEM(SkylightAmbient,               ShaderValueType::FLOAT) \
  ITEM(SkylightSpread,                ShaderValueType::FLOAT) \
  ITEM(SkylightSampleSpread,          ShaderValueType::FLOAT) \
  ITEM(RenderTargetSize,              ShaderValueType::VEC2) \
  ITEM(RenderTargetSizeRecip,         ShaderValueType::VEC2) \
  ITEM(ViewportSize,                  ShaderValueType::VEC2) \
  ITEM(PixelSize,                     ShaderValueType::VEC2) \
  ITEM(DiffuseColor,                  ShaderValueType::VEC4) \
  ITEM(AmbientColor,                  ShaderValueType::VEC4) \
  ITEM(DepthBias,                     ShaderValueType::FLOAT) \
  ITEM(GBufferSampleCount,            ShaderValueType::FLOAT) \
  ITEM(PPGaussPixelSize,              ShaderValueType::VEC2) \
  ITEM(PPGaussRelativeSize,           ShaderValueType::VEC2) \
  ITEM(PPDofEnabled,                  ShaderValueType::FLOAT) \
  ITEM(PPDofFocusDistance,            ShaderValueType::FLOAT) \
  ITEM(PPDofBlur,                     ShaderValueType::FLOAT) \
  ITEM(DirectToScreen,                ShaderValueType::FLOAT) \
  ITEM(DirectToSquare,                ShaderValueType::FLOAT) \

#define GLOBALSAMPLER_LIST \
  ITEM(SkylightTexture) \
  ITEM(DepthBufferSource) \
  ITEM(GBufferSourceA) \
  ITEM(SecondaryTexture) \
  ITEM(PPGauss) \
  ITEM(SquareTexture1) \
  ITEM(SquareTexture2) \

enum class GlobalUniformUsage {
#undef ITEM
#define ITEM(name, type) name,
  GLOBALUNIFORM_LIST
  LOCAL,	/// For non-global uniforms
};

enum class GlobalSamplerUsage {
#undef ITEM
#define ITEM(name) name,
  GLOBALSAMPLER_LIST
  LOCAL,	/// For non-global uniforms
};

extern const EnumMapperA GlobalUniformMapper[];
extern const EnumMapperA GlobalSamplerMapper[];
extern const ShaderValueType GlobalUniformTypes[];
extern const int GlobalUniformOffsets[];
extern const int GlobalSamplerOffsets[];

/// A struct to store global uniforms
struct Globals {
#undef ITEM
#define ITEM(name, type) ShaderValueTypes<type>::Type name;
  GLOBALUNIFORM_LIST

#undef ITEM
#define ITEM(name) shared_ptr<Texture> name;
  GLOBALSAMPLER_LIST
};

/// Stub parameter, becomes a slot
/// ":param vec4 MyColor" or ":param sampler2d MyTexture"
struct StubParameter {
  enum class Type {
    TVOID,
    FLOAT,
    VEC2,
    VEC3,
    VEC4,
    MATRIX44,
    SAMPLER2D,
    NONE,
  };

  Type mType;
  SharedString mName;

  static bool IsValidShaderValueType(Type type);
  static ShaderValueType ToShaderValueType(Type type);
};

/// Stub variables are outputs of a shader stage and inputs of the next shader stage.
/// ":output vec4 MyColor" -- creates "outMyColor" output variable.
/// ":input vec4 MyColor" -- creates "inMyColor" input variable.
struct StubInOutVariable {
  ShaderValueType type;
  string name;
};

/// Global uniforms
/// ":global vec2 gRenderTargetSize"
struct StubGlobalUniform {
  ShaderValueType type;
  string name;
  GlobalUniformUsage usage;
};

/// Global samplers
/// ":global sampler2D gSquareTexture"
struct StubGlobalSampler {
  string name;
  GlobalSamplerUsage usage;
  bool isMultiSampler; // type is "sampler2DMS"
  bool isShadow; // type is "sampler2DShadow"
};


/// All metadata collected from a stub source.
struct StubMetadata {
  StubMetadata(const string& name, StubParameter::Type returnType,
    const string& strippedSource,
    const vector<OWNERSHIP StubParameter*>& parameters,
    const vector<StubGlobalUniform*>& globalUniforms,
    const vector<StubGlobalSampler*>& globalSamplers,
    const vector<StubInOutVariable*>& inputs,
    const vector<StubInOutVariable*>& outputs);

  ~StubMetadata();

  /// Value of the ":name" directive.
  const string name;

  /// Value of the ":returns" directive.
  const StubParameter::Type returnType;

  /// The source without any directives.
  const string strippedSource;

  /// Parameters of the stub. These become slots. (":param")
  const vector<StubParameter*> parameters;

  /// List of globals. (":global")
  const vector<StubGlobalUniform*> globalUniforms;
  const vector<StubGlobalSampler*> globalSamplers;

  /// List of stage inputs. (":input")
  const vector<StubInOutVariable*> inputs;

  /// List of stage outputs. (":output")
  const vector<StubInOutVariable*> outputs;
};


/// A stub is a part of a shader. It contains shader source code annotated
/// by directives. Using these annotations, stubs can depend on each other and
/// on other nodes likes textures and floats. 
class StubNode : public Node {
  friend class ShaderBuilder;

public:
  StubNode();
  virtual ~StubNode();

  /// Returns the metadata containing information about the stub source.
  StubMetadata* GetStubMetadata() const;

  /// Get slot by shader parameter name
  Slot* GetSlotByParameter(StubParameter*);
  Slot* GetSlotByParameterName(const string& name);

  /// Source of the stub
  StringSlot mSource;

  /// Copies shader source from another StubNode
  virtual void CopyFrom(const shared_ptr<Node>& node) override;

protected:
  /// Performs metadata analysis on the new stub source.
  virtual void Operate() override;

  /// Handle received messages
  virtual void HandleMessage(Message* message) override;

  /// Metadata
  StubMetadata* mMetadata;

  /// Maps stub parameters to stub slots
  map<StubParameter*, Slot*> mParameterSlotMap;
  map<string, Slot*> mParameterNameSlotMap;
};

typedef TypedSlot<StubNode> StubSlot;

ShaderValueType NodeToValueType(const shared_ptr<Node>& node);