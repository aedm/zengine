#pragma once

#include "../dom/node.h"
#include "../nodes/valuenodes.h"
#include "../resources/texture.h"
#include "valuetype.h"
#include <vector>
#include <map>

/// Macro list for global uniforms (name/usage, type)
#define GLOBAL_UNIFORM_LIST \
  ITEM(Time,                          ValueType::FLOAT) \
  ITEM(Camera,                        ValueType::MATRIX44) \
  ITEM(World,                         ValueType::MATRIX44) \
  ITEM(View,                          ValueType::MATRIX44) \
  ITEM(Projection,                    ValueType::MATRIX44) \
  ITEM(Transformation,                ValueType::MATRIX44) \
  ITEM(SkylightCamera,                ValueType::MATRIX44) \
  ITEM(SkylightProjection,            ValueType::MATRIX44) \
  ITEM(SkylightTransformation,        ValueType::MATRIX44) \
  ITEM(SkylightTextureSizeRecip,      ValueType::FLOAT) \
  ITEM(SkylightDirection,             ValueType::VEC3) \
  ITEM(SkylightColor,                 ValueType::VEC3) \
  ITEM(SkylightAmbient,               ValueType::FLOAT) \
  ITEM(SkylightSpread,                ValueType::FLOAT) \
  ITEM(SkylightSampleSpread,          ValueType::FLOAT) \
  ITEM(SkylightBias,                  ValueType::FLOAT) \
  ITEM(RenderTargetSize,              ValueType::VEC2) \
  ITEM(RenderTargetSizeRecip,         ValueType::VEC2) \
  ITEM(ViewportSize,                  ValueType::VEC2) \
  ITEM(PixelSize,                     ValueType::VEC2) \
  ITEM(DiffuseColor,                  ValueType::VEC4) \
  ITEM(AmbientColor,                  ValueType::VEC4) \
  ITEM(DepthBias,                     ValueType::FLOAT) \
  ITEM(GBufferSampleCount,            ValueType::FLOAT) \
  ITEM(PPGaussPixelSize,              ValueType::VEC2) \
  ITEM(PPGaussRelativeSize,           ValueType::VEC2) \
  ITEM(PPDofEnabled,                  ValueType::FLOAT) \
  ITEM(PPDofFocusDistance,            ValueType::FLOAT) \
  ITEM(PPDofBlur,                     ValueType::FLOAT) \
  ITEM(PPDofScale,                    ValueType::FLOAT) \
  ITEM(PPDofBleed,                    ValueType::FLOAT) \
  ITEM(DirectToScreen,                ValueType::FLOAT) \
  ITEM(DirectToSquare,                ValueType::FLOAT) \
  ITEM(FluidCurlAmount,               ValueType::FLOAT) \
  ITEM(FluidForceDamp,                ValueType::FLOAT) \
  ITEM(FluidDeltaTime,                ValueType::FLOAT) \
  ITEM(FluidPressureFade,             ValueType::FLOAT) \
  ITEM(FluidDissipation,              ValueType::FLOAT) \

#define GLOBAL_SAMPLER_LIST \
  ITEM(SkylightTexture) \
  ITEM(DepthBufferSource) \
  ITEM(GBufferSourceA) \
  ITEM(SecondaryTexture) \
  ITEM(PPGauss) \
  ITEM(SquareTexture1) \
  ITEM(SquareTexture2) \
  ITEM(FluidVelocity1) \
  ITEM(FluidVelocity2) \
  ITEM(FluidVelocity3) \
  ITEM(FluidColor) \
  ITEM(FluidCurl) \
  ITEM(FluidDivergence) \
  ITEM(FluidPressure) \

enum class GlobalUniformUsage {
#undef ITEM
#define ITEM(name, type) name,
  GLOBAL_UNIFORM_LIST
  LOCAL,	/// For non-global uniforms
};

enum class GlobalSamplerUsage {
#undef ITEM
#define ITEM(name) name,
  GLOBAL_SAMPLER_LIST
  LOCAL,	/// For non-global uniforms
};

extern const EnumMapA<GlobalUniformUsage> GlobalUniformMapper;
extern const EnumMapA<GlobalSamplerUsage> GlobalSamplerMapper;
extern const ValueType GlobalUniformTypes[];
extern const int GlobalUniformOffsets[];
extern const int GlobalSamplerOffsets[];

/// A struct to store global uniforms
struct Globals {
#undef ITEM
#define ITEM(name, type) ValueTypes<type>::Type name;
  GLOBAL_UNIFORM_LIST

#undef ITEM
#define ITEM(name) std::shared_ptr<Texture> name;
  GLOBAL_SAMPLER_LIST
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
    IMAGE2D,
    BUFFER,
    NONE,
  };

  Type mType;
  std::string mName;

  static bool IsValidValueType(Type type);
  static ValueType ToValueType(Type type);
};

/// Stub variables are outputs of a shader stage and inputs of the next shader stage.
/// ":output vec4 MyColor" -- creates "outMyColor" output variable.
/// ":input vec4 MyColor" -- creates "inMyColor" input variable.
struct StubInOutVariable {
  ValueType type;
  std::string name;
};

/// Global uniforms
/// ":global vec2 gRenderTargetSize"
struct StubGlobalUniform {
  ValueType type;
  std::string name;
  GlobalUniformUsage usage;
};

/// Global samplers
/// ":global sampler2D gSquareTexture"
struct StubGlobalSampler {
  std::string name;
  GlobalSamplerUsage usage;
  bool isMultiSampler; // type is "sampler2DMS"
  bool isShadow; // type is "sampler2DShadow"
};


/// All metadata collected from a stub source.
struct StubMetadata {
  StubMetadata(std::string name, StubParameter::Type returnType,
               std::string strippedSource,
               std::vector<OWNERSHIP StubParameter*> parameters,
               std::vector<StubGlobalUniform*> globalUniforms,
               std::vector<StubGlobalSampler*> globalSamplers,
               std::vector<StubInOutVariable*> inputs,
               std::vector<StubInOutVariable*> outputs);

  ~StubMetadata();

  /// Value of the ":name" directive.
  const std::string mName;

  /// Value of theű ":returns" directive.
  const StubParameter::Type mReturnType;

  /// The source without any directives.
  const std::string mStrippedSource;

  /// Parameters of the stub. These become slots. (":param")
  const std::vector<StubParameter*> mParameters;

  /// List of globals. (":global")
  const std::vector<StubGlobalUniform*> mGlobalUniforms;
  const std::vector<StubGlobalSampler*> mGlobalSamplers;

  /// List of stage inputs. (":input")
  const std::vector<StubInOutVariable*> mInputs;

  /// List of stage outputs. (":output")
  const std::vector<StubInOutVariable*> mOutputs;
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
  Slot* GetSlotByParameterName(const std::string& name);

  /// Source of the stub
  StringSlot mSource;

  /// Copies shader source from another StubNode
  void CopyFrom(const std::shared_ptr<Node>& node) override;

protected:
  /// Performs metadata analysis on the new stub source.
  void Operate() override;

  /// Handle received messages
  void HandleMessage(Message* message) override;

  /// Metadata
  StubMetadata* mMetadata;

  /// Maps stub parameters to stub slots
  std::map<StubParameter*, Slot*> mParameterSlotMap;
  std::map<std::string, Slot*> mParameterNameSlotMap;
};

typedef TypedSlot<StubNode> StubSlot;

ValueType NodeToValueType(const std::shared_ptr<Node>& node);