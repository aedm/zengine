#define _CRT_SECURE_NO_WARNINGS

#include <include/resources/mesh.h>
#include <include/render/drawingapi.h>

#include <assert.h>

VertexFormat* VertexPos::Format;
VertexFormat* VertexPosNorm::Format;
VertexFormat* VertexPosUVNorm::Format;
VertexFormat* VertexPosUV::Format;

VertexFormat::VertexFormat(UINT BinaryFormat)
{
	memset(AttributesArray, 0, sizeof(void*) * (UINT)VertexAttributeUsage::COUNT);

	this->BinaryFormat = BinaryFormat;
	int stride = 0;
	for (int i=0; BinaryFormat; BinaryFormat>>=1, i++)
	{
		if (BinaryFormat & 1)
		{
			VertexAttribute attrib;
			attrib.Usage = (VertexAttributeUsage)i;
			attrib.Size = gVariableByteSizes[(UINT)gVertexAttributeType[i]];
			attrib.Offset = stride;
			Attributes.push_back(attrib);
			stride += attrib.Size;
		}
	}

	for (UINT i=0; i<Attributes.size(); i++)
	{
		VertexAttribute& attrib = Attributes[i];

		/// AttributesArray points into the vector. Meh.
		AttributesArray[(UINT)attrib.Usage] = &attrib;
	}

	Stride = stride;
	//Declaration = TheDrawingAPI->CreateVertexDeclaration(Attributes);

	assert(Stride % 4 == 0);	/// Vertex structure size should always be mod4.
}

VertexFormat::~VertexFormat()
{
	//TheDrawingAPI->DestroyVertexDeclaration(Declaration);
}

//void VertexFormat::SetAttributeDefines( ShaderDefines& Defines )
//{
//	for (UINT i=0; i<Attributes.size(); i++)
//	{
//		Defines[VertexAttribute::GetAttribString(Attributes[i].Type)] = L"";
//	}
//}

bool VertexFormat::HasAttribute( VertexAttributeUsage Attrib )
{
	return (BinaryFormat & (1 << (UINT)Attrib)) != 0;
}

Mesh::Mesh()
{
	VertexCount = 0;
	VertexBufferSize = 0;
	VertexHandle = 0;

	IndexCount = 0;
	IndexHandle = 0;

	WireframeIndexCount = 0;
	WireframeIndexHandle = 0;
	
	Format = NULL;
}

Mesh::~Mesh()
{
	if (VertexHandle) TheDrawingAPI->DestroyVertexBuffer(VertexHandle);
	if (IndexHandle) TheDrawingAPI->DestroyIndexBuffer(IndexHandle);
	if (WireframeIndexHandle) TheDrawingAPI->DestroyIndexBuffer(WireframeIndexHandle);
}

void Mesh::AllocateVertices(VertexFormat* Format, UINT VertexCount)							
{
	this->Format = Format;
	this->VertexCount = VertexCount;

	UINT newBufferSize = Format->Stride * VertexCount;
	if (VertexBufferSize != newBufferSize)
	{
		VertexBufferSize = newBufferSize;

		if (VertexHandle) TheDrawingAPI->DestroyVertexBuffer(VertexHandle);
		VertexHandle = TheDrawingAPI->CreateVertexBuffer(newBufferSize);
	}
}

void Mesh::AllocateIndices(UINT IndexCount)
{
	if (this->IndexCount != IndexCount)
	{
		this->IndexCount = IndexCount;
		if (IndexHandle) TheDrawingAPI->DestroyIndexBuffer(IndexHandle);
		IndexHandle = IndexCount ? TheDrawingAPI->CreateIndexBuffer(IndexCount * sizeof(IndexEntry)) : NULL;
	}
}

void Mesh::AllocateWireframeIndices(UINT IndexCount)
{
	if (WireframeIndexCount != IndexCount)
	{
		WireframeIndexCount = IndexCount;
		if (WireframeIndexHandle) TheDrawingAPI->DestroyIndexBuffer(WireframeIndexHandle);
		WireframeIndexHandle = IndexCount ? TheDrawingAPI->CreateIndexBuffer(WireframeIndexCount) : NULL;
	}
}

void Mesh::UploadIndices(const IndexEntry* Indices)
{
	//TheDrawingAPI->UploadIndices(IndexHandle, IndexCount, Indices);
	void* mappedIndices = TheDrawingAPI->MapIndexBuffer(IndexHandle);
	memcpy(mappedIndices, Indices, IndexCount * sizeof(IndexEntry));
	TheDrawingAPI->UnMapIndexBuffer(IndexHandle);
}

void Mesh::UploadVertices(void* Vertices)
{
	void* mappedMesh = TheDrawingAPI->MapVertexBuffer(VertexHandle);
	memcpy(mappedMesh, Vertices, VertexCount * Format->Stride);
	TheDrawingAPI->UnMapVertexBuffer(VertexHandle);
}


void Mesh::UploadVertices( void* Vertices, int VertexCount )
{
	void* mappedMesh = TheDrawingAPI->MapVertexBuffer(VertexHandle);
	memcpy(mappedMesh, Vertices, VertexCount * Format->Stride);
	TheDrawingAPI->UnMapVertexBuffer(VertexHandle);
}

//const wchar_t* VertexAttribute::GetAttribString( VertexAttributeType Type )
//{
//	return Framework::GetWStringFromEnum(AttributeTypeMapper,  Type);
//}