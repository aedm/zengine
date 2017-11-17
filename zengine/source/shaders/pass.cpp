#include "shaderbuilder.h"
#include <include/shaders/pass.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadersource.h>
#include <include/shaders/enginestubs.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>

REGISTER_NODECLASS(Pass, "Pass");

static SharedString VertesStubSlotName = make_shared<string>("Vertex shader");
static SharedString FragmentStubSlotName = make_shared<string>("Fragment shader");
static SharedString UberShaderStubSlotName = make_shared<string>("Uber Shader");
static SharedString FaceModeSlotName = make_shared<string>("Face mode");
static SharedString BlendModeSlotName = make_shared<string>("Blending");

Pass::Pass()
  : Node(NodeType::PASS)
  , mVertexStub(this, VertesStubSlotName)
  , mFragmentStub(this, FragmentStubSlotName)
  , mFaceModeSlot(this, FaceModeSlotName)
  , mBlendModeSlot(this, BlendModeSlotName)
  , mUberShader(this, UberShaderStubSlotName, false, false, false)
  , mHandle(-1) 
{
  mRenderstate.mDepthTest = true;
  mRenderstate.mFaceMode = RenderState::FaceMode::FRONT;
  mRenderstate.mBlendMode = RenderState::BlendMode::ALPHA;
  mUberShader.Connect(TheEngineStubs->GetStub("uber"));
}

Pass::~Pass() 
{
  RemoveUniformSlots();
}

void Pass::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
    case MessageType::VALUE_CHANGED:
      if (message->mSlot == &mVertexStub || message->mSlot == &mFragmentStub || 
          message->mSlot == &mUberShader) {
        SafeDelete(mVertexShaderMetadata);
        SafeDelete(mFragmentShaderMetadata);
        mIsUpToDate = false;
      } 
      TheMessageQueue.Enqueue(nullptr, this, MessageType::NEEDS_REDRAW);
      break;
    default: break;
  }
}

void Pass::Operate() {
  mUniforms.clear();
  mSamplers.clear();
  mAttributes.clear();
  /// TODO: delete previous handle
  mHandle = -1;
  RemoveUniformSlots();

  /// Recreates shader source
  if (mShaderSource == nullptr) {
    mShaderSource = ShaderBuilder::FromStubs(mVertexStub.GetNode(),  
                                             mFragmentStub.GetNode());
    if (mShaderSource == nullptr) return;
  }
  
  INFO("Building render pipeline...");

  for (auto sampler : mVertexShaderMetadata->mSamplers) {
    if (sampler->mNode) {
      Slot* slot = new Slot(sampler->mNode->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(sampler->mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto uniform : mVertexShaderMetadata->mUniforms) {
    if (uniform->mNode) {
      Slot* slot = new Slot(uniform->mNode->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(uniform->mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto sampler : mFragmentShaderMetadata->mSamplers) {
    if (sampler->mNode) {
      Slot* slot = new Slot(sampler->mNode->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(sampler->mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto uniform : mFragmentShaderMetadata->mUniforms) {
    if (uniform->mNode) {
      Slot* slot = new Slot(uniform->mNode->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(uniform->mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }

  const string& vertexSource = mVertexShaderMetadata->mSource;
  const string& fragmentSource = mFragmentShaderMetadata->mSource;

  INFO("Building shader program...");
  ShaderProgram* shaderCompileDesc = OpenGL->CreateShaderFromSource(
    vertexSource.c_str(), fragmentSource.c_str());
  if (shaderCompileDesc == nullptr) {
    ERR("Missing shader compilation result.");
    return;
  }

  mHandle = shaderCompileDesc->Handle;

  /// Collect uniforms from shader stage sources
  map<string, ShaderSource::Uniform*> uniformMap;
  for (auto uniform : mVertexShaderMetadata->mUniforms) {
    uniformMap[uniform->mName] = uniform;
  }
  for (auto uniform : mFragmentShaderMetadata->mUniforms) {
    uniformMap[uniform->mName] = uniform;
  }

  /// Collect samplers
  map<string, ShaderSource::Uniform*> samplerMap;
  for (auto sampler : mVertexShaderMetadata->mSamplers) {
    samplerMap[sampler->mName] = sampler;
  }
  for (auto sampler : mFragmentShaderMetadata->mSamplers) {
    samplerMap[sampler->mName] = sampler;
  }

  /// Merge uniform info
  for (auto uniform : shaderCompileDesc->Uniforms) {
    ShaderSource::Uniform* sourceUniform = uniformMap.at(uniform.mName);
    PassUniform passUniform;
    passUniform.handle = uniform.Handle;
    passUniform.node = sourceUniform->mNode;
    passUniform.globalType = sourceUniform->mGlobalType;
    passUniform.type = sourceUniform->mType;
    mUniforms.push_back(passUniform);
  }

  /// Merge sampler info
  for (auto sampler : shaderCompileDesc->Samplers) {
    ShaderSource::Uniform* sourceUniform = samplerMap.at(sampler.mName);
    PassUniform passUniform;
    passUniform.handle = sampler.mHandle;
    passUniform.node = sourceUniform->mNode;
    passUniform.globalType = sourceUniform->mGlobalType;
    passUniform.type = sourceUniform->mType;
    mSamplers.push_back(passUniform);
  }

  /// Collect required attributes
  for (auto attributeDesc : shaderCompileDesc->Attributes) {
    mAttributes.push_back(attributeDesc);
  }
}

void Pass::Set(Globals* globals) {
  ASSERT(mHandle != -1);

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
  OpenGL->SetShaderProgram(mHandle);

  /// Set uniforms
  for (PassUniform& uniform : mUniforms) {
    if (uniform.globalType == ShaderGlobalType::LOCAL) {
      /// Local uniform, takes value from a slot
      ASSERT(uniform.node != nullptr);
      switch (uniform.type) {
#undef ITEM
#define ITEM(name, type) \
				case NodeType::name: { \
          auto vNode = static_cast<ValueNode<NodeType::name>*>(uniform.node); \
          vNode->Update(); \
					OpenGL->SetUniform(uniform.handle, NodeType::name, &vNode->Get()); } \
					break;
        VALUETYPE_LIST

        default: SHOULD_NOT_HAPPEN; break;
      }
    } else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalUniformOffsets[(UINT)uniform.globalType];
      void* source = reinterpret_cast<char*>(globals)+offset;
      OpenGL->SetUniform(uniform.handle, uniform.type, source);
    }
  }

  /// Set samplers
  UINT i = 0;
  for (PassUniform& sampler : mSamplers) {
    Texture* tex = nullptr;
    if (sampler.globalType == ShaderGlobalType::LOCAL) {
      ASSERT(sampler.node != nullptr);
      tex = static_cast<TextureNode*>(sampler.node)->Get();
    } else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalUniformOffsets[(UINT)sampler.globalType];
      void* source = reinterpret_cast<char*>(globals) + offset;
      tex = *reinterpret_cast<Texture**>(source);
    }
    OpenGL->SetTexture(sampler.handle, tex ? tex->mHandle : 0, i++,
                              tex ? tex->mIsMultisampe : false);
  }
}

const vector<ShaderProgram::Attribute>& Pass::GetUsedAttributes() {
  return mAttributes;
}

bool Pass::isComplete() {
  return mHandle != -1;
}

void Pass::RemoveUniformSlots() {
  for (Slot* slot : mUniformAndSamplerSlots) delete slot;
  mUniformAndSamplerSlots.clear();
}
