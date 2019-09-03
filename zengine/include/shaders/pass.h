#pragma once

#include "shadersource.h"
#include "../dom/node.h"
#include "../render/drawingapi.h"
#include <map>

using namespace std;

enum class PassType {
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

  void Collect(const vector<SourceType>& sources, const vector<ProgramType>& targets) {
    mResources.clear();
    map<string, const SourceType*> mapByName;
    for (const auto& source : sources) {
      mapByName[source.mName] = &source;
    }
    for (auto& target : targets) {
      auto it = mapByName.find(target.mName);
      ASSERT(it != mapByName.end());
      mResources.push_back(Item(it->second, &target));
    }
  }

  const vector<Item> GetResources() {
    return mResources;
  }

private:
  vector<Item> mResources;
};


/// A renderpass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
/// It manages the entire render pipeline, including setting a shader program,
/// render states, and connecting pipeline resources.
class Pass: public Node {
public:
  Pass();

  /// Shader stages and sources
  StubSlot mVertexStub;
  StubSlot mFragmentStub;
  StubSlot mUberShader;

  /// Pipeline state
  FloatSlot mFaceModeSlot;
  FloatSlot mBlendModeSlot;
  RenderState mRenderstate;

  void Set(Globals* globals);

  /// Returns true if pass can be used
  bool isComplete();

  /// Get shader sources
  string GetVertexShaderSource();
  string GetFragmentShaderSource();

protected:
  virtual void HandleMessage(Message* message) override;

  /// Creates the shader program
  virtual void Operate() override;

  void BuildShaderSource();

  /// Generated shader source
  shared_ptr<ShaderSource> mShaderSource;
  
  /// Compiled and linked shader program
  shared_ptr<ShaderProgram> mShaderProgram;

  /// Mapping between nodes and shader resources
  ShaderResourceMap<ShaderProgram::Uniform, ShaderSource::Uniform> mUniforms;
  ShaderResourceMap<ShaderProgram::Sampler, ShaderSource::Sampler> mSamplers;
  ShaderResourceMap<ShaderProgram::SSBO, ShaderSource::NamedResource> mSSBOs;

  /// Client-side uniform buffer. 
  /// Uniforms are assembled in this array and then uploaded to OpenGL
  shared_ptr<Buffer> mUniformBuffer = make_shared<Buffer>();
};

typedef TypedSlot<Pass> PassSlot;
