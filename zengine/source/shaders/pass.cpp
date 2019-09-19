#include "shaderbuilder.h"
#include <include/shaders/pass.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadersource.h>
#include <include/shaders/enginestubs.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/texturenode.h>
#include <include/nodes/buffernode.h>

REGISTER_NODECLASS(Pass, "Pass");

const int MAX_UNIFORM_BUFFER_SIZE = 4096;

Pass::Pass()
  : Node()
  , mVertexStub(this, "Vertex shader")
  , mFragmentStub(this, "Fragment shader")
  , mFaceModeSlot(this, "Face mode")
  , mBlendModeSlot(this, "Blending")
  , mFluidSourceSlot(this, "Fluid source", true)
  , mFluidColorTargetSlot(this, "Fluid color target", true)
  , mFluidVelocityTargetSlot(this, "Fluid velo target", true)
  , mUberShader(this, "Uber Shader", false, false, false)
{
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
  mShaderProgram.reset();
  if (!mShaderSource) return;

  const string& vertexSource = mShaderSource->mVertexSource;
  const string& fragmentSource = mShaderSource->mFragmentSource;

  INFO("Building shader program...");
  mShaderProgram =
    OpenGL->CreateShaderFromSource(vertexSource.c_str(), fragmentSource.c_str());
  if (mShaderProgram == nullptr) {
    ERR("Missing shader compilation result.");
    return;
  }

  mUniforms.Collect(mShaderSource->mUniforms, mShaderProgram->mUniforms);
  mSamplers.Collect(mShaderSource->mSamplers, mShaderProgram->mSamplers);
  mSSBOs.Collect(mShaderSource->mSSBOs, mShaderProgram->mSSBOs);

  /// Allocate space for uniform array
  ASSERT(mShaderProgram->mUniformBlockSize <= MAX_UNIFORM_BUFFER_SIZE);
  mUniformBuffer->Allocate(mShaderProgram->mUniformBlockSize);
}

void Pass::BuildShaderSource()
{
  mIsUpToDate = false;

  /// Generate shader source
  mShaderSource.reset();
  mShaderSource = 
    ShaderBuilder::FromStubs(mVertexStub.GetNode(), mFragmentStub.GetNode());
  if (!mShaderSource) {
    mShaderProgram.reset();
  }
}

void Pass::Set(Globals* globals) {
  Update();
  if (!mShaderProgram) return;

  RenderState::FaceMode faceMode = RenderState::FaceMode::FRONT;
  const float faceVal = mFaceModeSlot.Get();
  if (faceVal > 0.666f) faceMode = RenderState::FaceMode::BACK;
  else if (faceVal > 0.333f) faceMode = RenderState::FaceMode::FRONT_AND_BACK;
  mRenderstate.mFaceMode = faceMode;

  RenderState::BlendMode blendMode = RenderState::BlendMode::ALPHA;
  const float blendVal = mBlendModeSlot.Get();
  if (blendVal > 0.666f) blendMode = RenderState::BlendMode::NORMAL;
  else if (blendVal > 0.333f) blendMode = RenderState::BlendMode::ADDITIVE;
  mRenderstate.mBlendMode = blendMode;

  OpenGL->SetRenderState(&mRenderstate);

  if (mFluidSourceSlot.GetMultiNodeCount() > 0) {
    auto& fluidSource = PointerCast<FluidNode>(mFluidSourceSlot.GetReferencedMultiNode(0));
    fluidSource->SetGlobalFluidTextures(globals);
  }
  if (mFluidColorTargetSlot.GetMultiNodeCount() > 0) {
    auto& fluid = PointerCast<FluidNode>(mFluidColorTargetSlot.GetReferencedMultiNode(0));
    fluid->SetColorRenderTarget();
  }
  if (mFluidVelocityTargetSlot.GetMultiNodeCount() > 0) {
    auto& fluid = PointerCast<FluidNode>(mFluidVelocityTargetSlot.GetReferencedMultiNode(0));
    fluid->SetVelocityRenderTarget();
  }

  char uniformArray[MAX_UNIFORM_BUFFER_SIZE];

  /// Fill uniform array item by item, take value from Nodes and put them
  /// into the array using the uniform offset. Not particularly nice code,
  /// but fast enough.
  for (const auto& uniformMapper : mUniforms.GetResources()) {
    const ShaderSource::Uniform* source = uniformMapper.mSource;
    const ShaderProgram::Uniform* target = uniformMapper.mTarget;
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
      const int offset = GlobalUniformOffsets[(UINT)source->mGlobalType];
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

  mUniformBuffer->UploadData(uniformArray, mShaderProgram->mUniformBlockSize);
  OpenGL->SetShaderProgram(mShaderProgram, mUniformBuffer);

  /// Set samplers
  UINT i = 0;
  for (const auto& samplerMapper : mSamplers.GetResources()) {
    const ShaderSource::Sampler* source = samplerMapper.mSource;
    const ShaderProgram::Sampler* target = samplerMapper.mTarget;

    shared_ptr<Texture> tex = nullptr;
    if (samplerMapper.mSource->mGlobalType == GlobalSamplerUsage::LOCAL) {
      ASSERT(samplerMapper.mSource->mNode != nullptr);
      tex = PointerCast<TextureNode>(samplerMapper.mSource->mNode)->Get();
    }
    else {
      /// Global uniform, takes value from the Globals object
      const int offset = GlobalSamplerOffsets[(UINT)source->mGlobalType];
      void* sourcePointer = reinterpret_cast<char*>(globals) + offset;
      tex = *reinterpret_cast<shared_ptr<Texture>*>(sourcePointer);
    }
    OpenGL->SetTexture(*target, tex ? tex : nullptr, i++);
  }

  /// Set SSBOs
  for (const auto& ssbo : mSSBOs.GetResources()) {
    const ShaderSource::NamedResource* source = ssbo.mSource;
    const ShaderProgram::SSBO* target = ssbo.mTarget;
    shared_ptr<Buffer> buffer = 
      PointerCast<BufferNode>(ssbo.mSource->mNode)->GetBuffer();
    
    if (!buffer) continue;
    OpenGL->SetSSBO(target->mIndex, buffer);
  }
}

bool Pass::isComplete() const
{
  return (mShaderProgram != nullptr);
}

std::string Pass::GetVertexShaderSource() const
{
  return mShaderSource ? mShaderSource->mVertexSource : "[No vertex shader]";
}

std::string Pass::GetFragmentShaderSource() const
{
  return mShaderSource ? mShaderSource->mFragmentSource : "[No fragment shader]";
}
