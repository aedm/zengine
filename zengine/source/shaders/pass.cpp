#include "shaderbuilder.h"
#include <include/shaders/pass.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadersource.h>
#include <include/shaders/enginestubs.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/texturenode.h>

REGISTER_NODECLASS(Pass, "Pass");

const int MAX_UNIFORM_BUFFER_SIZE = 4096;

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
  if (!mShaderSource) return;
  mShaderProgram.reset();

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

  /// Generate shader source
  mShaderSource.reset();
  mShaderSource = ShaderBuilder::FromStubs(mVertexStub.GetNode(),
    mFragmentStub.GetNode());
  if (!mShaderSource) return;

  INFO("Building render pipeline...");

  for (auto& sampler : mShaderSource->mSamplers) {
    if (sampler.mNode) {
      shared_ptr<Slot> slot =
        make_shared<TextureSlot>(this, string(), false, false, false, false);
      slot->Connect(sampler.mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto& uniform : mShaderSource->mUniforms) {
    if (uniform.mNode) {
      shared_ptr<Slot> slot = shared_ptr<Slot>(CreateValueSlot(
        NodeToValueType(uniform.mNode), this, string(), false, false, false, false));
      slot->Connect(uniform.mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
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
      int offset = GlobalSamplerOffsets[(UINT)source->mGlobalType];
      void* sourcePointer = reinterpret_cast<char*>(globals) + offset;
      tex = *reinterpret_cast<shared_ptr<Texture>*>(sourcePointer);
    }
    OpenGL->SetTexture(*target, tex ? tex : nullptr, i++);
  }

  /// Set SSBOs
  //NOT_IMPLEMENTED;
}

bool Pass::isComplete() {
  return (mShaderProgram != nullptr);
}

std::string Pass::GetVertexShaderSource() {
  return mShaderSource ? mShaderSource->mVertexSource : "[No vertex shader]";
}

std::string Pass::GetFragmentShaderSource() {
  return mShaderSource ? mShaderSource->mFragmentSource : "[No fragment shader]";
}
