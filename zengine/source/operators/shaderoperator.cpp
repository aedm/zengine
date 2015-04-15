#include <include/render/drawingapi.h>
#include <include/operators/shaderoperator.h>
#include <include/base/helpers.h>
#include <include/resources/texture.h>
#include <include/shaders/shaderBuilder.h>
#include <boost/foreach.hpp>

ShaderOperator::ShaderOperator( Shader* _Shader )
	: Node(string("Shader"))
	, ShaderProgram(_Shader)
{
	Type = OP_SHADER;
	Init();
}

ShaderOperator::ShaderOperator( const ShaderOperator& Original )
	: Node(Original)
	, ShaderProgram(Original.ShaderProgram)
{
	Init();
}


void ShaderOperator::Init()
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


void ShaderOperator::Set()
{
	ASSERT(IsProperlyConnected);

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
		Texture* const texture = slot.SourceSlot->Value();
		if (texture)
		{
			TheDrawingAPI->SetTexture(slot.TargetSampler->Handle, texture->Handle, i);
		} else {
			ERR("Null texture defined for sampler.");
		}
	}
}

void ShaderOperator::OnSlotConnectionsChanged( Slot* S )
{
	RegenerateCopyItems();
}

void ShaderOperator::RegenerateCopyItems()
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
		if (item.TargetUniform->IsLocal && item.SourceSlot->GetConnectedNode() == NULL)
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
			case name: \
				source = reinterpret_cast<const UINT*>(&ToValueSlot<name>(slot)->Value()); \
				break;
				NODETYPE_LIST

			default: NOT_IMPLEMENTED; break;
			}
			UINT* target = reinterpret_cast<UINT*>(
				reinterpret_cast<char*>(item.Array->Values) + 
				item.TargetUniform->ByteOffset + item.ByteOffset);
			UINT floatCount = VariableByteSizes[slot->GetType()] / sizeof(float);
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
			UINT floatCount = VariableByteSizes[global->Type] / sizeof(float);
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
	vector<Slot*>& oSlots = Owner->Slots;
	foreach(Uniform* uniform, Block->Uniforms)
	{
		if (uniform->IsLocal)
		{
			LocalUniform* local = static_cast<LocalUniform*>(uniform);
			if (local->Desc->Elements)
			{
				int numSlots = VariableByteSizes[UINT(local->Type)] / sizeof(float);
				for (int i=0; i<numSlots; i++)
				{
					FloatSlot* slot = new FloatSlot(Owner, (*local->Desc->Elements)[i].UIName);
					oUniformMap.push_back(UniformMapping(local, i * sizeof(float), this, slot));
					oSlots.push_back(slot);
				}
			} else {
				Slot* slot = NULL;
				switch(local->Type)
				{
				#undef ITEM
				#define ITEM(name, type, token) \
				case name: slot = new TypedSlot<name>(Owner, local->Desc->UniformName); break;
				NODETYPE_LIST

				default: SHOULDNT_HAPPEN; break;
				}
				oSlots.push_back(slot);
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

TextureSlot* ShaderOperator::GetSamplerSlotByName( const char* Name )
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

void ShaderOperator::AttachToSampler(const char* Name, Node* Op)
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

const vector<UniformMapping>& ShaderOperator::GetUniforms()
{
	return Uniforms;
}

ShaderOperator::~ShaderOperator()
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

Node* ShaderOperator::Clone() const
{
	return new ShaderOperator(*this);
}

UniformArray::CopyItem::CopyItem( const UINT* _Source, UINT* _Target, bool _IsGlobal )
	: Source(_Source)
	, Target(_Target)
	, IsGlobal(_IsGlobal)
{}

void ShaderOperator::AttachToUniform(const char* Name, Node* Op, int FloatIndex)
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
