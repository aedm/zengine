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
  : mVertexStub(this, VertesStubSlotName)
  , mFragmentStub(this, FragmentStubSlotName)
  , mFaceModeSlot(this, FaceModeSlotName)
  , mBlendModeSlot(this, BlendModeSlotName)
  , mUberShader(this, UberShaderStubSlotName, false, false, false) {
  mRenderstate.mDepthTest = true;
  mRenderstate.mFaceMode = RenderState::FaceMode::FRONT;
  mRenderstate.mBlendMode = RenderState::BlendMode::ALPHA;
  mUberShader.Connect(TheEngineStubs->GetStub("uber"));
}

Pass::~Pass() {
  RemoveUniformSlots();
}

void Pass::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
    case MessageType::VALUE_CHANGED:
      if (message->mSlot == &mVertexStub || message->mSlot == &mFragmentStub ||
          message->mSlot == &mUberShader) {
        mIsUpToDate = false;
      }
      TheMessageQueue.Enqueue(nullptr, this, MessageType::NEEDS_REDRAW);
      break;
    default: break;
  }
}

void Pass::Operate() {
  /// Clean up from previous state
  mUsedUniforms.clear();
  mUsedSamplers.clear();
  mShaderProgram.reset();
  mShaderSource.reset();
  RemoveUniformSlots();

  /// Generate shader source
  if (!mShaderSource) {
    mShaderSource = ShaderBuilder::FromStubs(mVertexStub.GetNode(),
                                             mFragmentStub.GetNode());
    if (!mShaderSource) return;
  }
  //INFO("--------------- VERTEX SHADER ----------------");
  //INFO(mShaderSource->mVertexSource.c_str());
  //INFO("--------------- FRAGMENT SHADER ----------------");
  //INFO(mShaderSource->mFragmentSource.c_str());

  INFO("Building render pipeline...");

  for (auto& sampler : mShaderSource->mSamplers) {
    if (sampler.mNode) {
      Slot* slot = new TextureSlot(this, nullptr, false, false, false, false);
      slot->Connect(sampler.mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }
  for (auto uniform : mShaderSource->mUniforms) {
    if (uniform.mNode) {
      Slot* slot = CreateValueSlot(uniform.mNode->GetValueType(), 
        this, nullptr, false, false, false, false);
      slot->Connect(uniform.mNode);
      mUniformAndSamplerSlots.push_back(slot);
    }
  }

  const string& vertexSource = mShaderSource->mVertexSource;
  const string& fragmentSource = mShaderSource->mFragmentSource;

  INFO("Building shader program...");
  mShaderProgram =
    OpenGL->CreateShaderFromSource(vertexSource.c_str(), fragmentSource.c_str());
  if (mShaderProgram == nullptr) {
    ERR("Missing shader compilation result.");
    return;
  }

  /// Collect uniforms and samplers from shader stage sources
  map<string, const ShaderSource::Uniform*> uniformMap;
  for (auto& uniform : mShaderSource->mUniforms) {
    uniformMap[uniform.mName] = &uniform;
  }
  map<string, const ShaderSource::Sampler*> samplerMap;
  for (auto& sampler : mShaderSource->mSamplers) {
    samplerMap[sampler.mName] = &sampler;
  }

  /// Create list of used uniforms and samplers
  for (auto& uniform : mShaderProgram->mUniforms) {
    auto it = uniformMap.find(uniform.mName);
    ASSERT(it != uniformMap.end());
    mUsedUniforms.push_back(UniformMapper(&uniform, it->second));
  }
  for (auto& sampler : mShaderProgram->mSamplers) {
    auto it = samplerMap.find(sampler.mName);
    ASSERT(it != samplerMap.end());
    mUsedSamplers.push_back(SamplerMapper(&sampler, it->second));
  }

  /// Allocate space for uniform array
  mUniformArray.resize(mShaderProgram->mUniformBlockSize);
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

  /// Fill uniform array item by item, take value from Nodes and put them
  /// into the array using the uniform offset. Not particularly nice code,
  /// but fast enough.
  for (UniformMapper& uniformMapper : mUsedUniforms) {
    const ShaderSource::Uniform* source = uniformMapper.mSource;
    const ShaderProgram::Uniform* target = uniformMapper.mTarget;
    if (source->mGlobalType == ShaderGlobalType::LOCAL) {
      /// Local uniform, takes value from a slot
      ASSERT(source->mNode != nullptr);
      switch (source->mType) {
        #undef ITEM
        #define ITEM(name, capitalizedName, type) \
				case ValueType::name: { \
          auto vNode = SafeCast<ValueNode<ValueType::name>*>(source->mNode); \
          vNode->Update(); \
          *(reinterpret_cast<type*>(&mUniformArray[target->mOffset])) = vNode->Get(); \
					break; \
        }
        VALUETYPE_LIST
        default: SHOULD_NOT_HAPPEN; break;
      }
    } else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalUniformOffsets[(UINT)source->mGlobalType];
      switch (source->mType) {
        #undef ITEM
        #define ITEM(name, capitalizedName, type) \
				case ValueType::name: { \
          void* valuePointer = reinterpret_cast<char*>(globals)+offset; \
          *(reinterpret_cast<type*>(&mUniformArray[target->mOffset])) = \
            *reinterpret_cast<type*>(valuePointer); \
					break; \
        }
        VALUETYPE_LIST
        default: SHOULD_NOT_HAPPEN; break;
      }
    }
  }

  OpenGL->SetShaderProgram(mShaderProgram, &mUniformArray[0]);
  
  /// Set samplers
  UINT i = 0;
  for (SamplerMapper& samplerMapper : mUsedSamplers) {
    const ShaderSource::Sampler* source = samplerMapper.mSource;
    const ShaderProgram::Sampler* target = samplerMapper.mTarget;

    Texture* tex = nullptr;
    if (samplerMapper.mSource->mGlobalType == ShaderGlobalType::LOCAL) {
      ASSERT(samplerMapper.mSource->mNode != nullptr);
      tex = static_cast<TextureNode*>(samplerMapper.mSource->mNode)->Get();
    } else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalUniformOffsets[(UINT)source->mGlobalType];
      void* sourcePointer = reinterpret_cast<char*>(globals) + offset;
      tex = *reinterpret_cast<Texture**>(sourcePointer);
    }
    OpenGL->SetTexture(*target, tex ? tex->mHandle : 0, i++,
                       tex ? tex->mIsMultisampe : false);
  }
}

const vector<ShaderProgram::Attribute>& Pass::GetUsedAttributes() {
  return mShaderProgram->mAttributes;
}

bool Pass::isComplete() {
  return (mShaderProgram != nullptr);
}

void Pass::RemoveUniformSlots() {
  for (Slot* slot : mUniformAndSamplerSlots) delete slot;
  mUniformAndSamplerSlots.clear();
}

Pass::UniformMapper::UniformMapper(const ShaderProgram::Uniform* target,
                                   const ShaderSource::Uniform* source)
  : mTarget(target)
  , mSource(source) {}

Pass::SamplerMapper::SamplerMapper(const ShaderProgram::Sampler* target,
                                   const ShaderSource::Sampler* source)
  : mTarget(target)
  , mSource(source) {}
