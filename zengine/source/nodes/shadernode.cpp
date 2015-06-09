#include <include/render/drawingapi.h>
#include <include/nodes/shadernode.h>
#include <include/base/helpers.h>
#include <include/resources/texture.h>
#include <include/shaders/shaderBuilder.h>
#include <boost/foreach.hpp>


ShaderSlot::ShaderSlot(Node* Owner, SharedString Name)
	: Slot(NodeType::SHADER, Owner, Name)
{}

void ShaderSlot::Set() const
{
	ShaderNode* node = static_cast<ShaderNode*>(GetNode());
	if (node != nullptr) node->Set();
}


ShaderNode::ShaderNode( Shader* _Shader )
	: Node(NodeType::SHADER, string("Shader"))
	, ShaderProgram(_Shader)
{
	Init();
}

ShaderNode::ShaderNode( const ShaderNode& Original )
	: Node(Original)
	, ShaderProgram(Original.ShaderProgram)
{
	Init();
}


void ShaderNode::Init()
{
	UniformArrays.reserve(ShaderProgram->UniformBlocks.size());
	foreach(UniformBlock* block, ShaderProgram->UniformBlocks)
	{
		UniformArray* uniformArray = new UniformArray(block);
		uniformArray->CreateSlotsAndMappings(this, Uniforms);
		UniformArrays.push_back(uniformArray);
	}
	foreach(Sampler* sampler, ShaderProgram->Samplers)
	{
		TextureSlot* textureSlot = new TextureSlot(this, sampler->Desc->UniformName);
		Samplers.push_back(SamplerMapping(sampler, textureSlot));
	}
	RegenerateCopyItems();
}


void ShaderNode::Set()
{
	ASSERT(isProperlyConnected);

	ShaderProgram->Set();

	/// Lazy evaluation of slots
	Evaluate();

	/// Upload uniforms
	foreach(UniformArray* uniformArray, UniformArrays)
	{
		uniformArray->Set();
	}

	/// Set textures
	for (UINT i=0; i<Samplers.size(); i++)
	{
		SamplerMapping& slot = Samplers[i];
		Texture* const texture = slot.SourceSlot->Get();
		if (texture)
		{
			TheDrawingAPI->SetTexture(slot.TargetSampler->Handle, texture->Handle, i);
		} else {
			ERR("Null texture defined for sampler.");
		}
	}
}

void ShaderNode::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		RegenerateCopyItems();
		break;
	default:
		break;
	}
}

void ShaderNode::RegenerateCopyItems()
{
	/// Reset copy items
	foreach(UniformArray* uniformArray, UniformArrays)
	{
		uniformArray->CopyItems.clear();
	}

	/// Check whether everything is connected
	bool allConnected = true;
	foreach(UniformMapping& item, Uniforms)
	{
		if (item.TargetUniform->IsLocal && item.SourceSlot->GetNode() == NULL)
		{
			allConnected = false;
			break;
		}
	}

	/// Don't generate anything if there's a connection mismatch
	if (!allConnected) return;

	/// Reserve space for copy items
	foreach(UniformArray* uniformArray, UniformArrays)
	{
		uniformArray->CopyItems.reserve(uniformArray->Block->ByteSize / sizeof(float));
	}
	
	/// Regenerate copy items
	foreach(UniformMapping& item, Uniforms)
	{
		if (item.TargetUniform->IsLocal)
		{
			Slot* slot = item.SourceSlot;
			const UINT* source = NULL;
			switch(item.SourceSlot->GetType())
			{
				#undef ITEM
				#define ITEM(name, type, token) \
			case NodeType::name: \
				source = reinterpret_cast<const UINT*>(&ToValueSlot<NodeType::name>(slot)->Get()); \
				break;
				VALUETYPE_LIST

			default: NOT_IMPLEMENTED; break;
			}
			UINT* target = reinterpret_cast<UINT*>(
				reinterpret_cast<char*>(item.Array->Values) + 
				item.TargetUniform->ByteOffset + item.ByteOffset);
			UINT floatCount = gVariableByteSizes[(UINT)slot->GetType()] / sizeof(float);
			for (UINT i=0; i<floatCount; i++)
			{
				item.Array->CopyItems.push_back(
					UniformArray::CopyItem(source + i, target + i, false));
			}
		} else {
			GlobalUniform* global = static_cast<GlobalUniform*>(item.TargetUniform);
			const UINT* source = reinterpret_cast<const UINT*>(GlobalUniformOffsets[global->Usage]);
			UINT* target = reinterpret_cast<UINT*>(
				reinterpret_cast<char*>(item.Array->Values) + global->ByteOffset);
			UINT floatCount = gVariableByteSizes[(UINT)global->Type] / sizeof(float);
			for (UINT i=0; i<floatCount; i++)
			{
				item.Array->CopyItems.push_back(
					UniformArray::CopyItem(source + i, target + i, true));
			}
		}
	}
}


void UniformArray::CreateSlotsAndMappings( Node* Owner, vector<UniformMapping>& oUniformMap )
{
	foreach(Uniform* uniform, Block->Uniforms)
	{
		if (uniform->IsLocal)
		{
			LocalUniform* local = static_cast<LocalUniform*>(uniform);
			if (local->Desc->Elements)
			{
				int numSlots = gVariableByteSizes[UINT(local->Type)] / sizeof(float);
				for (int i=0; i<numSlots; i++)
				{
					FloatSlot* slot = new FloatSlot(Owner, (*local->Desc->Elements)[i].UIName);
					oUniformMap.push_back(UniformMapping(local, i * sizeof(float), this, slot));
				}
			} else {
				Slot* slot = NULL;
				switch(local->Type)
				{
				#undef ITEM
				#define ITEM(name, type, token) \
				case NodeType::name: slot = new ValueSlot<NodeType::name>(Owner, local->Desc->UniformName); break;
				VALUETYPE_LIST

				default: SHOULDNT_HAPPEN; break;
				}
				oUniformMap.push_back(UniformMapping(local, 0, this, slot));
			}
		} else {
			GlobalUniform* global = static_cast<GlobalUniform*>(uniform);
			oUniformMap.push_back(UniformMapping(global, 0, this, NULL));
		}
	}
}

UniformArray::UniformArray( UniformBlock* _Block )
	: Block(_Block)
{
	Values = new char[Block->ByteSize];
}

UniformArray::~UniformArray()
{
	SafeDelete(Values);
}

void UniformArray::Set()
{
	/// Collect values
	foreach (CopyItem& copyItem, CopyItems) 
	{
		if (copyItem.IsGlobal) 
		{ 
			char* source = reinterpret_cast<char*>(&TheGlobals) + reinterpret_cast<UINT>(copyItem.Source);
			*(copyItem.Target) = *(reinterpret_cast<UINT*>(source));
		} else {
			*(copyItem.Target) = *(copyItem.Source);
		}
	}

	/// Upload values
	foreach(Uniform* uniform, Block->Uniforms)
	{
		TheDrawingAPI->SetUniform(uniform->Handle, uniform->Type, 
			reinterpret_cast<char*>(Values) + uniform->ByteOffset);
	}
}

UniformMapping::UniformMapping( Uniform* _TargetUniform, UINT _ByteOffset, 
	UniformArray* _Array, Slot* _SourceSlot )
	: TargetUniform(_TargetUniform)
	, ByteOffset(_ByteOffset)
	, Array(_Array)
	, SourceSlot(_SourceSlot)
{}

SamplerMapping::SamplerMapping( Sampler* _TargetSampler, TextureSlot* _SourceSlot )
	: TargetSampler(_TargetSampler)
	, SourceSlot(_SourceSlot)
{}

TextureSlot* ShaderNode::GetSamplerSlotByName( const char* Name )
{
	foreach (SamplerMapping& p, Samplers)
	{
		if (p.TargetSampler->Desc != NULL && 
			p.TargetSampler->Desc->UniformName->compare(Name) == 0) 
		{
			return p.SourceSlot;
		}
	}
	ERR("There is no sampler called '%s'.", Name);
	return NULL;
}

void ShaderNode::AttachToSampler(const char* Name, Node* Op)
{
	if (this == NULL)
	{
		ERR("Shader doesn't exist.");
		return;
	}

	foreach (SamplerMapping& p, Samplers)
	{
		if (p.TargetSampler->Desc != NULL && p.TargetSampler->Desc->UniformName->compare(Name) == 0) 
		{
			TextureSlot* slot = p.SourceSlot;
			slot->Connect(Op);
			return;
		}
	}
	WARN("There is no sampler called '%s'.", Name);
}

const vector<UniformMapping>& ShaderNode::GetUniforms()
{
	return Uniforms;
}

ShaderNode::~ShaderNode()
{
	foreach (SamplerMapping& item, Samplers)
	{
		SafeDelete(item.SourceSlot);
	}

	foreach(UniformMapping& item, Uniforms)
	{
		SafeDelete(item.SourceSlot);
	}
}

Node* ShaderNode::Clone() const
{
	return new ShaderNode(*this);
}

UniformArray::CopyItem::CopyItem( const UINT* _Source, UINT* _Target, bool _IsGlobal )
	: Source(_Source)
	, Target(_Target)
	, IsGlobal(_IsGlobal)
{}

void ShaderNode::AttachToUniform(const char* Name, Node* Op, int FloatIndex)
{
	if (this == NULL)
	{
		ERR("Shader doesn't exist.");
		return;
	}

	foreach (UniformMapping& p, Uniforms)
	{
		if (!p.TargetUniform->IsLocal) continue;

		LocalUniform* local = static_cast<LocalUniform*>(p.TargetUniform);
		if (FloatIndex == (p.ByteOffset / sizeof(float)) &&
			local->Desc != NULL && local->Desc->UniformName->compare(Name) == 0) 
		{
			Slot* slot = p.SourceSlot;
			if (slot->GetType() == Op->GetType())
			{
				slot->Connect(Op);
				return;
			} else {
				ERR("Uniform '%s' has a different slot type than the operator.", Name);
			}

			/// There was an error, clear slot
			slot->Connect(NULL);
			return;
		}
	}
	WARN("There is no local uniform called '%s'.", Name);
}
