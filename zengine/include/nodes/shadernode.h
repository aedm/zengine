#pragma once

#include "../dom/node.h"
#include "../shaders/shaders.h"
#include "../nodes/valuenodes.h"

class UniformArray;
class Texture;
struct UniformMapping;
struct SamplerMapping;


class ShaderSlot: public Slot
{
public:
	ShaderSlot(Node* Owner, SharedString Name);

	void						Set() const;
};


class ShaderNode: public Node
{
public:
	ShaderNode(Shader* _Shader);
	ShaderNode(const ShaderNode& Original);

	virtual ~ShaderNode();

	/// Returns a slot created for a certain sampler
	TextureSlot*				GetSamplerSlotByName(const char* Name);

	/// Attaches an operator to a uniform
	void						AttachToUniform(const char* Name, Node* Op, 
		int FloatIndex = 0);

	void						AttachToSampler(const char* Name, Node* Op);

	/// Collects uniforms values and sets the shader active
	void						Set();
	
	const vector<UniformMapping>& GetUniforms();

	/// Clone shader operator
	virtual Node*	Clone() const;

	/// TODO: use shared_ptr
	Shader* const				ShaderProgram;

protected:
	vector<UniformArray*>		UniformArrays;

	/// Map from slots to uniforms
	vector<UniformMapping>		Uniforms;

	/// Map from slots to uniforms
	vector<SamplerMapping>		Samplers;

	/// Hook to do fancy stuff when the slots of the operator changed
	virtual void				HandleMessage(Slot* S, NodeMessage Message, const void* Payload) override;

	/// Recreates source->destination pointer pairs for collecting uniforms
	void						RegenerateCopyItems();

	/// Initialize object (UniformsArrays and Samplers)
	void						Init();
};


/// Uniforms to slots mapping. If a local uniform has "separate floats", each of
/// them has its own UniformMapping instance.
struct UniformMapping
{
	Uniform*					TargetUniform;
	UniformArray*				Array;

	/// Data source for local uniforms, otherwise NULL
	Slot*						SourceSlot;

	/// For uniforms with separate floats this is the offset 
	/// inside the vector value, otherwise zero.
	UINT						ByteOffset;

	UniformMapping(Uniform* TargetUniform, UINT ByteOffset, UniformArray* Array, 
		OWNERSHIP Slot* SourceSlot);
};


/// Samplers to slots mapping
struct SamplerMapping
{
	Sampler*					TargetSampler;
	TextureSlot*				SourceSlot;

	SamplerMapping(Sampler* TargetSampler, OWNERSHIP TextureSlot* SourceSlot);
};


/// Array of uniforms. Connects the operator to the shader by 
/// mapping between slots and uniforms.
class UniformArray
{
public:
	UniformArray(UniformBlock* Block);
	~UniformArray();

	/// Creates slots for an operator
	void						CreateSlotsAndMappings(Node* Owner, 
		vector<UniformMapping>& oSlotMap);

	/// Uploads uniform values through the graphics API
	void						Set();

	/// Client-side array for storing uniform values. 
	void*						Values;

	/// Uniform block descriptor
	UniformBlock*				Block;

	/// Fast local uniform collection: we store pointers of values inside slots and
	/// pointers inside value array. Upon collection we simply copy them in 32 bit units.
	struct CopyItem
	{
		CopyItem(const UINT* Source, UINT* Target, bool IsGlobal);
		const UINT*				Source;
		UINT*					Target;
		bool					IsGlobal;
	};

	vector<CopyItem>			CopyItems;
};


