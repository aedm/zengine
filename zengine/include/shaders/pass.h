#pragma once

#include "shadersource.h"
#include "../dom/node.h"
#include "../render/drawingapi.h"
#include "../nodes/fluidnode.h"
#include <map>
#include <memory>

enum class PassType {
  FLUID_PAINT,
  SHADOW,
  SOLID,
  ZPOST,
};

/// Stores a map between Nodes and shader program resources (eg. uniforms, buffers, etc)
template <typename ProgramType, typename SourceType>
class ShaderResourceMap {
public:
  struct Item {
    const SourceType* mSource;
    const ProgramType* mTarget;
    Item(const SourceType* source, const ProgramType* target)
      : mSource(source)
      , mTarget(target) 
    {}
  };

  void Collect(const std::vector<SourceType>& sources, 
    const std::vector<ProgramType>& targets)
  {
    mResources.clear();
    std::map<std::string, const SourceType*> mapByName;
    for (const auto& source : sources) {
      mapByName[source.mName] = &source;
    }
    for (auto& target : targets) {
      auto it = mapByName.find(target.mName);
      ASSERT(it != mapByName.end());
      mResources.push_back(Item(it->second, &target));
    }
  }

  const std::vector<Item>& GetResources() {
    return mResources;
  }

private:
  std::vector<Item> mResources;
};


/// A render pass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
/// It manages the entire render pipeline, including setting a shader program,
/// render states, and connecting pipeline resources.
class Pass: public Node {
public:
  Pass();

  /// Shader stages and sources
  StubSlot mVertexStub;
  StubSlot mFragmentStub;

  /// Pipeline state
  FloatSlot mFaceModeSlot;
  FloatSlot mBlendModeSlot;
  FluidSlot mFluidSourceSlot;
  FluidSlot mFluidColorTargetSlot;
  FluidSlot mFluidVelocityTargetSlot;

  /// Uber shader
  StubSlot mUberShader;
  
  RenderState mRenderstate{};

  void Set(Globals* globals);

  /// Returns true if pass can be used
  bool isComplete() const;

  /// Get shader sources
  std::string GetVertexShaderSource() const;
  std::string GetFragmentShaderSource() const;

protected:
  void HandleMessage(Message* message) override;

  /// Creates the shader program
  void Operate() override;

  void BuildShaderSource();

  /// Generated shader source
  std::shared_ptr<ShaderSource> mShaderSource;
  
  /// Compiled and linked shader program
  std::shared_ptr<ShaderProgram> mShaderProgram;

  /// Mapping between nodes and shader resources
  ShaderResourceMap<ShaderProgram::Uniform, ShaderSource::Uniform> mUniforms;
  ShaderResourceMap<ShaderProgram::Sampler, ShaderSource::Sampler> mSamplers;
  ShaderResourceMap<ShaderProgram::SSBO, ShaderSource::NamedResource> mSSBOs;

  /// Client-side uniform buffer. 
  /// Uniforms are assembled in this array and then uploaded to OpenGL
  std::shared_ptr<Buffer> mUniformBuffer = std::make_shared<Buffer>();
};

typedef TypedSlot<Pass> PassSlot;
