#include <include/shaders/pass.h>
#include <include/shaders/shaderstub.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>

REGISTER_NODECLASS(Pass, "Pass");

static SharedString VertesStubSlotName = make_shared<string>("Vertex shader");
static SharedString FragmentStubSlotName = make_shared<string>("Fragment shader");

Pass::Pass()
  : Node(NodeType::PASS)
  , mVertexStub(this, VertesStubSlotName)
  , mFragmentStub(this, FragmentStubSlotName)
  , mVertexSource(this, nullptr, false, false)
  , mFragmentSource(this, nullptr, false, false)
  , mHandle(-1) 
{}

Pass::~Pass() 
{}

void Pass::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      if (slot == &mVertexStub) {
        if (mVertexStub.GetNode() == nullptr) {
          mVertexSource.Connect(nullptr);
        } else {
          mVertexSource.Connect(mVertexStub.GetNode()->GetShaderSource());
        }
      } else if (slot == &mFragmentStub) {
        if (mFragmentStub.GetNode() == nullptr) {
          mFragmentSource.Connect(nullptr);
        } else {
          mFragmentSource.Connect(mFragmentStub.GetNode()->GetShaderSource());
        }
      } else if (slot == &mVertexSource || slot == &mFragmentSource) {
        BuildRenderPipeline();
      }
      break;
    case NodeMessage::VALUE_CHANGED:
      if (slot == &mVertexSource || slot == &mFragmentSource) {
        BuildRenderPipeline();
      }
      break;
    default: break;
  }
}

void Pass::BuildRenderPipeline() {
  mUniforms.clear();
  mSamplers.clear();
  mAttributes.clear();
  /// TODO: delete previous handle
  mHandle = -1;

  ShaderSource2* vertex = mVertexSource.GetNode();
  ShaderSource2* fragment = mFragmentSource.GetNode();
  if (vertex == nullptr || vertex->GetMetadata() == nullptr ||
      fragment == nullptr || fragment->GetMetadata() == nullptr) {
    return;
  }

  INFO("Building render pipeline...");

  const string& vertexSource = vertex->GetSource();
  const string& fragmentSource = fragment->GetSource();

  ShaderCompileDesc* shaderCompileDesc = TheDrawingAPI->CreateShaderFromSource(
    vertexSource.c_str(), fragmentSource.c_str());
  if (shaderCompileDesc == nullptr) {
    ERR("Missing shader compilation result.");
    return;
  }

  mHandle = shaderCompileDesc->Handle;

  /// Collect uniforms from shader stage sources
  map<string, ShaderSourceUniform*> uniformMap;
  for (auto sampler : vertex->GetMetadata()->uniforms) {
    uniformMap[sampler->name] = sampler;
  }
  for (auto sampler : fragment->GetMetadata()->uniforms) {
    uniformMap[sampler->name] = sampler;
  }

  /// Collect samplers
  map<string, ShaderSourceUniform*> samplerMap;
  for (auto sampler : vertex->GetMetadata()->samplers) {
    samplerMap[sampler->name] = sampler;
  }
  for (auto sampler : fragment->GetMetadata()->samplers) {
    samplerMap[sampler->name] = sampler;
  }

  /// Merge uniform info
  for (auto samplerDesc : shaderCompileDesc->Uniforms) {
    ShaderSourceUniform* sourceUniform = uniformMap.at(samplerDesc.Name);
    PassUniform passUniform;
    passUniform.handle = samplerDesc.Handle;
    passUniform.node = sourceUniform->node;
    passUniform.globalType = sourceUniform->globalType;
    passUniform.type = sourceUniform->type;
    mUniforms.push_back(passUniform);
  }

  /// Merge sampler info
  for (auto samplerDesc : shaderCompileDesc->Samplers) {
    ShaderSourceUniform* sourceUniform = samplerMap.at(samplerDesc.Name);
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

  TheDrawingAPI->SetShaderProgram(mHandle);

  /// Set uniforms
  for (PassUniform& uniform : mUniforms) {
    if (uniform.globalType == ShaderGlobalType::LOCAL) {
      /// Local uniform, takes value from a slot
      ASSERT(uniform.node != nullptr);
      switch (uniform.type) {
#undef ITEM
#define ITEM(name, type, token) \
				case NodeType::name: \
					TheDrawingAPI->SetUniform(uniform.handle, NodeType::name, \
						&static_cast<ValueNode<NodeType::name>*>(uniform.node)->Get()); \
					break;
        VALUETYPE_LIST

        default: SHOULDNT_HAPPEN; break;
      }
    } else {
      /// Global uniform, takes value from the Globals object
      int offset = GlobalUniformOffsets[(UINT)uniform.globalType];
      void* source = reinterpret_cast<char*>(globals)+offset;
      TheDrawingAPI->SetUniform(uniform.handle, uniform.type, source);
    }
  }

  /// Set samplers
  UINT i = 0;
  for (PassUniform& sampler : mSamplers) {
    ASSERT(sampler.node != nullptr);
    Texture* tex = static_cast<TextureNode*>(sampler.node)->Get();
    TheDrawingAPI->SetTexture(sampler.handle, tex ? tex->mHandle : 0, i++);
  }
}

const vector<ShaderAttributeDesc>& Pass::GetUsedAttributes() {
  return mAttributes;
}

Node* Pass::Clone() const {
  return new Pass();
}

bool Pass::isComplete() {
  return mHandle != -1;
}

const Slot* Pass::GetFragmentSourceSlot() {
  return &mFragmentSource;
}

const Slot* Pass::GetVertexSourceSlot() {
  return &mVertexSource;
}

