#include "shaderbuilder.h"
#include <include/shaders/pass.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadersource.h>
#include <include/shaders/enginestubs.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/texturenode.h>


REGISTER_NODECLASS(Pass, "Pass");

Pass::Pass()
  : Node()
  , mVertexStub(this, "Vertex shader")
  , mFragmentStub(this, "Fragment shader")
  , mFaceModeSlot(this, "Face mode")
  , mBlendModeSlot(this, "Blending")
  , mUberShader(this, "Uber Shader", false, false, false) {
  mRenderstate.mDepthTest = true;
  mRenderstate.mFaceMode = RenderState::FaceMode::FRONT;
  mRenderstate.mBlendMode = RenderState::BlendMode::ALPHA;
  mUberShader.Connect(TheEngineStubs->GetStub("uber"));
}

void Pass::HandleMessage(Message* message) {
  switch (message->mType) {
  case MessageType::SLOT_CONNECTION_CHANGED:
  case MessageType::VALUE_CHANGED:
    if (message->mSlot == &mVertexStub || message->mSlot == &mFragmentStub ||
      message->mSlot == &mUberShader) {
      BuildShaderSource();
    }
    EnqueueMessage(MessageType::NEEDS_REDRAW);
    break;
  default: break;
  }
}

void Pass::Operate() {
  /// Clean up from previous state
  if (!mVertexShaderSource || !mFragmentShaderSource) return;
  mShaderProgram.reset();
  mUsedSamplers.clear();
  mUsedVertexUniforms.clear();
  mUsedFragmentUniforms.clear();

  const string& vertexSource = mVertexShaderSource->mSource;
  const string& fragmentSource = mFragmentShaderSource->mSource;

  INFO("Building shader program...");
  mShaderProgram =
    OpenGL->CreateShaderFromSource(vertexSource.c_str(), fragmentSource.c_str());
  if (mShaderProgram == nullptr) {
    ERR("Missing shader compilation result.");
    return;
  }

  CollectUniformsAndSamplersFromShaderStage(mShaderProgram->mVertexStage, 
    mUsedVertexUniforms, mUsedSamplers);
  CollectUniformsAndSamplersFromShaderStage(mShaderProgram->mFragmentStage,
    mUsedFragmentUniforms, mUsedSamplers);

  /// Allocate space for uniform array
  mVertexUniformArray.resize(mShaderProgram->mVertexStage->mUniformBlockSize);
  mFragmentUniformArray.resize(mShaderProgram->mFragmentStage->mUniformBlockSize);
}

void Pass::CollectUniformsAndSamplersFromShaderStage(
  const shared_ptr<ShaderCompiledStage>& shaderStage, vector<UniformMapper>& uniforms, 
  vector<SamplerMapper>& samplers)
{
  map<string, const ShaderSource::Uniform*> uniformMap;
  for (auto& uniform : mVertexShaderSource->mUniforms) {
    uniformMap[uniform.mName] = &uniform;
  }
  map<string, const ShaderSource::Sampler*> samplerMap;
  for (auto& sampler : mVertexShaderSource->mSamplers) {
    samplerMap[sampler.mName] = &sampler;
  }
  /// Create list of used uniforms and samplers
  for (auto& uniform : shaderStage->mUniforms) {
    auto it = uniformMap.find(uniform.mName);
    ASSERT(it != uniformMap.end());
    uniforms.push_back(UniformMapper(&uniform, it->second));
  }
  for (auto& sampler : shaderStage->mSamplers) {
    auto it = samplerMap.find(sampler.mName);
    ASSERT(it != samplerMap.end());
    samplers.push_back(SamplerMapper(&sampler, it->second));
  }
}

Slot* CreateValueSlot(ValueType type, Node* owner,
  const string& name, bool isMultiSlot = false,
  bool isPublic = true, bool isSerializable = true,
  float minimum = 0.0f, float maximum = 1.0f) 
{
  switch (type)
  {
  case ValueType::FLOAT:
    return new FloatSlot(owner, name, isMultiSlot, isPublic, isSerializable, minimum, maximum);
  case ValueType::VEC2:
    return new Vec2Slot(owner, name, isMultiSlot, isPublic, isSerializable, minimum, maximum);
  case ValueType::VEC3:
    return new Vec3Slot(owner, name, isMultiSlot, isPublic, isSerializable, minimum, maximum);
  case ValueType::VEC4:
    return new Vec4Slot(owner, name, isMultiSlot, isPublic, isSerializable, minimum, maximum);
  case ValueType::MATRIX44:
    return new MatrixSlot(owner, name, isMultiSlot, isPublic, isSerializable, minimum, maximum);
  default:
    SHOULD_NOT_HAPPEN;
    return nullptr;
  }
}

void Pass::BuildShaderSource()
{
  mIsUpToDate = false;

  /// Reset slots
  mUniformAndSamplerSlots.clear();
  ClearSlots();
  AddSlot(&mVertexStub, true, true, true);
  AddSlot(&mFragmentStub, true, true, true);
  AddSlot(&mFaceModeSlot, true, true, true);
  AddSlot(&mBlendModeSlot, true, true, true);

  /// Generate shader sources
  mFragmentShaderSource.reset();
  mVertexShaderSource.reset();

  auto& vertexStubNode = mVertexStub.GetNode();
  auto& fragmentStubNode = mFragmentStub.GetNode();
  if (!vertexStubNode || !fragmentStubNode) return;

  mFragmentShaderSource = ShaderBuilder::GenerateFragmentShaderSource(fragmentStubNode);
  if (!mFragmentShaderSource) return;
  mVertexShaderSource = ShaderBuilder::GenerateVertexShaderSource(vertexStubNode);
  if (!mVertexShaderSource) return;

  INFO("Building render pipeline...");

  //for (auto& sampler : mShaderSource->mSamplers) {
  //  if (sampler.mNode) {
  //    shared_ptr<Slot> slot =
  //      make_shared<TextureSlot>(this, string(), false, false, false, false);
  //    slot->Connect(sampler.mNode);
  //    mUniformAndSamplerSlots.push_back(slot);
  //  }
  //}
  //for (auto& uniform : mShaderSource->mUniforms) {
  //  if (uniform.mNode) {
  //    shared_ptr<Slot> slot = shared_ptr<Slot>(CreateValueSlot(
  //      NodeToValueType(uniform.mNode), this, string(), false, false, false, false));
  //    slot->Connect(uniform.mNode);
  //    mUniformAndSamplerSlots.push_back(slot);
  //  }
  //}
}

void Pass::Set(Globals* globals) {
  Update();
  if (!mShaderProgram) return;

  RenderState::FaceMode faceMode = RenderState::FaceMode::FRONT;
  float faceVal = mFaceModeSlot.Get();
  if (faceVal > 0.666f) faceMode = RenderState::FaceMode::BACK;
  else if (faceVal > 0.333f) faceMode = RenderState::FaceMode::FRONT_AND_BACK;
  mRenderstate.mFaceMode = faceMode;

  RenderState::BlendMode blendMode = RenderState::BlendMode::ALPHA;
  float blendVal = mBlendModeSlot.Get();
  if (blendVal > 0.666f) blendMode = RenderState::BlendMode::NORMAL;
  else if (blendVal > 0.333f) blendMode = RenderState::BlendMode::ADDITIVE;
  mRenderstate.mBlendMode = blendMode;

  OpenGL->SetRenderState(&mRenderstate);

  FillUniformArray(mUsedVertexUniforms, globals, mVertexUniformArray);
  FillUniformArray(mUsedFragmentUniforms, globals, mFragmentUniformArray);

  OpenGL->SetShaderProgram(mShaderProgram, 
    &mVertexUniformArray[0], &mFragmentUniformArray[0]);

  /// Set samplers
  UINT i = 0;
  for (SamplerMapper& samplerMapper : mUsedSamplers) {
    const ShaderSource::Sampler* source = samplerMapper.mSource;
    const ShaderCompiledStage::Sampler* target = samplerMapper.mTarget;

    shared_ptr<Texture> tex = nullptr;
    if (samplerMapper.mSource->mGlobalType == GlobalSamplerUsage::LOCAL) {
      ASSERT(samplerMapper.mSource->mNode != nullptr);
      tex = PointerCast<TextureNode>(samplerMapper.mSource->mNode)->Get();
    }
    else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalSamplerOffsets[(UINT)source->mGlobalType];
      void* sourcePointer = reinterpret_cast<char*>(globals) + offset;
      tex = *reinterpret_cast<shared_ptr<Texture>*>(sourcePointer);
    }
    OpenGL->SetTexture(*target, tex ? tex : nullptr, i++);
  }
}


void Pass::FillUniformArray(vector<UniformMapper>& uniforms, Globals* globals,
  vector<char>& uniformArray)
{
  /// Fill uniform array item by item, take value from Nodes and put them
  /// into the array using the uniform offset. Not particularly nice code,
  /// but fast enough.
  for (UniformMapper& uniformMapper : uniforms) {
    const ShaderSource::Uniform* source = uniformMapper.mSource;
    const ShaderCompiledStage::Uniform* target = uniformMapper.mTarget;
    if (source->mGlobalType == GlobalUniformUsage::LOCAL) {
      /// Local uniform, takes value from a slot
      ASSERT(source->mNode != nullptr);
      switch (source->mType) {
#undef ITEM
#define ITEM(name) \
				  case name: { \
            auto& vNode = \
              PointerCast<ValueNode<ValueTypes<name>::Type>>(source->mNode); \
            vNode->Update(); \
            *(reinterpret_cast<ValueTypes<name>::Type*>( \
              &uniformArray[target->mOffset])) = vNode->Get(); \
					  break; \
          }
        ITEM(ValueType::FLOAT);
        ITEM(ValueType::VEC2);
        ITEM(ValueType::VEC3);
        ITEM(ValueType::VEC4);
        ITEM(ValueType::MATRIX44);
      default: SHOULD_NOT_HAPPEN; break;
      }
    }
    else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalUniformOffsets[(UINT)source->mGlobalType];
      switch (source->mType) {
#undef ITEM
#define ITEM(name) \
        case name: { \
          void* valuePointer = reinterpret_cast<char*>(globals)+offset; \
          *(reinterpret_cast<ValueTypes<name>::Type*>( \
            &uniformArray[target->mOffset])) = \
            *reinterpret_cast<ValueTypes<name>::Type*>(valuePointer); \
          break; \
        }
        ITEM(ValueType::FLOAT);
        ITEM(ValueType::VEC2);
        ITEM(ValueType::VEC3);
        ITEM(ValueType::VEC4);
        ITEM(ValueType::MATRIX44);
      default: SHOULD_NOT_HAPPEN; break;
      }
    }
  }
}


bool Pass::isComplete() {
  return (mShaderProgram != nullptr);
}

Pass::UniformMapper::UniformMapper(const ShaderCompiledStage::Uniform* target,
  const ShaderSource::Uniform* source)
  : mTarget(target)
  , mSource(source) {}

Pass::SamplerMapper::SamplerMapper(const ShaderCompiledStage::Sampler* target,
  const ShaderSource::Sampler* source)
  : mTarget(target)
  , mSource(source) {}
