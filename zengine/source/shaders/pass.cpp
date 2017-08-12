#include "shaderbuilder.h"
#include <include/shaders/pass.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadernode.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>

REGISTER_NODECLASS(Pass, "Pass");

static SharedString VertesStubSlotName = make_shared<string>("Vertex shader");
static SharedString FragmentStubSlotName = make_shared<string>("Fragment shader");
static SharedString FaceModeSlotName = make_shared<string>("Face mode");
static SharedString BlendModeSlotName = make_shared<string>("Blending");

Pass::Pass()
  : Node(NodeType::PASS)
  , mVertexStub(this, VertesStubSlotName)
  , mFragmentStub(this, FragmentStubSlotName)
  , mFaceModeSlot(this, FaceModeSlotName)
  , mBlendModeSlot(this, BlendModeSlotName)
  , mHandle(-1) 
{
  mRenderstate.mDepthTest = true;
  mRenderstate.mFaceMode = RenderState::FaceMode::FRONT;
  mRenderstate.mBlendMode = RenderState::BlendMode::ALPHA;
}

Pass::~Pass() 
{
  RemoveUniformSlots();
}

void Pass::HandleMessage(NodeMessage message, Slot* slot) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::VALUE_CHANGED:
      if (slot == &mVertexStub || slot == &mFragmentStub) {
        SafeDelete(mVertexShaderMetadata);
        SafeDelete(mFragmentShaderMetadata);
        mIsUpToDate = false;
      } 
      TheMessageQueue.Enqueue(this, NodeMessage::NEEDS_REDRAW);
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

  /// Recreates shader metadata if needed
  if (mVertexShaderMetadata == nullptr) {
    mVertexShaderMetadata = ShaderBuilder::FromStub(mVertexStub.GetNode(), "vertex");
  }
  if (mFragmentShaderMetadata == nullptr) {
    mFragmentShaderMetadata = ShaderBuilder::FromStub(mFragmentStub.GetNode(), "fragment");
  }

  /// Both fragment and vertex metadata are needed
  if (mFragmentShaderMetadata == nullptr || mVertexShaderMetadata == nullptr) {
    return;
  }
  
  INFO("Building render pipeline...");

  for (auto sampler : mVertexShaderMetadata->mSamplers) {
    if (sampler->node) {
      Slot* slot = new Slot(sampler->node->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(sampler->node);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto uniform : mVertexShaderMetadata->mUniforms) {
    if (uniform->node) {
      Slot* slot = new Slot(uniform->node->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(uniform->node);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto sampler : mFragmentShaderMetadata->mSamplers) {
    if (sampler->node) {
      Slot* slot = new Slot(sampler->node->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(sampler->node);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto uniform : mFragmentShaderMetadata->mUniforms) {
    if (uniform->node) {
      Slot* slot = new Slot(uniform->node->GetType(), this, nullptr, false, false, false, false);
      slot->Connect(uniform->node);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }

  const string& vertexSource = mVertexShaderMetadata->mSource;
  const string& fragmentSource = mFragmentShaderMetadata->mSource;

  ShaderCompileDesc* shaderCompileDesc = OpenGL->CreateShaderFromSource(
    vertexSource.c_str(), fragmentSource.c_str());
  if (shaderCompileDesc == nullptr) {
    ERR("Missing shader compilation result.");
    return;
  }

  mHandle = shaderCompileDesc->Handle;

  /// Collect uniforms from shader stage sources
  map<string, ShaderUniform*> uniformMap;
  for (auto sampler : mVertexShaderMetadata->mUniforms) {
    uniformMap[sampler->name] = sampler;
  }
  for (auto sampler : mFragmentShaderMetadata->mUniforms) {
    uniformMap[sampler->name] = sampler;
  }

  /// Collect samplers
  map<string, ShaderUniform*> samplerMap;
  for (auto sampler : mVertexShaderMetadata->mSamplers) {
    samplerMap[sampler->name] = sampler;
  }
  for (auto sampler : mFragmentShaderMetadata->mSamplers) {
    samplerMap[sampler->name] = sampler;
  }

  /// Merge uniform info
  for (auto samplerDesc : shaderCompileDesc->Uniforms) {
    ShaderUniform* sourceUniform = uniformMap.at(samplerDesc.Name);
    PassUniform passUniform;
    passUniform.handle = samplerDesc.Handle;
    passUniform.node = sourceUniform->node;
    passUniform.globalType = sourceUniform->globalType;
    passUniform.type = sourceUniform->type;
    mUniforms.push_back(passUniform);
  }

  /// Merge sampler info
  for (auto samplerDesc : shaderCompileDesc->Samplers) {
    ShaderUniform* sourceUniform = samplerMap.at(samplerDesc.Name);
    PassUniform passUniform;
    passUniform.handle = samplerDesc.Handle;
    passUniform.node = sourceUniform->node;
    passUniform.globalType = sourceUniform->globalType;
    passUniform.type = sourceUniform->type;
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

const vector<ShaderAttributeDesc>& Pass::GetUsedAttributes() {
  return mAttributes;
}

bool Pass::isComplete() {
  return mHandle != -1;
}

void Pass::RemoveUniformSlots() {
  for (Slot* slot : mUniformAndSamplerSlots) delete slot;
  mUniformAndSamplerSlots.clear();
}
