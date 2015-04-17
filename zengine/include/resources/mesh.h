#pragma once

#include "../base/defines.h"
#include "../base/vectormath.h"
#include "../render/drawingapi.h"
#include <vector>

using namespace std;

class ResourceManager;
class VertexFormat;

enum VertexAttributeMask
{
	VERTEXATTRIB_POSITION_MASK		= 1 << (UINT)VertexAttributeUsage::POSITION,
	VERTEXATTRIB_TEXCOORD_MASK		= 1 << (UINT)VertexAttributeUsage::TEXCOORD,
	VERTEXATTRIB_NORMAL_MASK		= 1 << (UINT)VertexAttributeUsage::NORMAL,
	VERTEXATTRIB_BINORMAL_MASK		= 1 << (UINT)VertexAttributeUsage::BINORMAL,
	//VERTEXATTRIB_COLOR_MASK			= 1 << VERTEXATTRIB_COLOR
};


/// Common vertex formats
struct VertexPos
{
	Vec3						Position;

	/// Vertex format descriptor for this struct
	static VertexFormat*		Format;
};

struct VertexPosNorm
{
	Vec3						Position;
	Vec3						Normal;

	/// Vertex format descriptor for this struct
	static VertexFormat*		Format;
};

struct VertexPosUVNorm
{
	Vec3						Position;
	Vec2						UV;
	Vec3						Normal;

	/// Vertex format descriptor for this struct
	static VertexFormat*		Format;
};

struct VertexPosUV
{
	Vec3						Position;
	Vec2						UV;

	/// Vertex format descriptor for this struct
	static VertexFormat*		Format;
};


class VertexFormat
{
	friend class ResourceManager;

	VertexFormat(UINT BinaryFormat);
	~VertexFormat();

public:
	void						Set();
	//void						SetAttributeDefines(ShaderDefines& Defines);

	bool						HasAttribute(VertexAttributeUsage Attrib);

	//VertexDeclaration			Declaration;
	int							Stride;
	UINT						BinaryFormat;
	vector<VertexAttribute>		Attributes;

	VertexAttribute*			AttributesArray[(UINT)VertexAttributeUsage::COUNT];
};


class Mesh
{
	friend class ResourceManager;

	Mesh();
	~Mesh();

public:
	void						AllocateVertices(VertexFormat* Format, UINT VertexCount);
	void						AllocateIndices(UINT IndexCount);
	void						AllocateWireframeIndices(UINT IndexCount);

	/// Uploads all vertices
	void						UploadVertices(void* Vertices);

	/// Uploads only the first VertexCount vertices, doessn't reallocate
	void						UploadVertices(void* Vertices, int VertexCount);

	/// Uploads all indices
	void						UploadIndices(const IndexEntry* Indices);

	template<typename T, int N>	void SetVertices(const T (&StaticVertices)[N]);
	template<int N>				void SetIndices(const IndexEntry (&StaticIndices)[N]);

	UINT						VertexCount;
	UINT						VertexBufferSize;
	VertexBufferHandle			VertexHandle;

	UINT						IndexCount;
	IndexBufferHandle			IndexHandle;

	UINT						WireframeIndexCount;
	IndexBufferHandle			WireframeIndexHandle;

	VertexFormat*				Format;
};

template<typename T, int N>
void Mesh::SetVertices(const T (&StaticVertices)[N])
{
	AllocateVertices(T::Format, N);
	UploadVertices((void*)StaticVertices);
}

template<int N>
void Mesh::SetIndices(const IndexEntry (&StaticIndices)[N])
{
	AllocateIndices(N);
	UploadIndices(StaticIndices);
}
