#define _CRT_SECURE_NO_WARNINGS

#include <include/resources/mesh.h>
#include <include/render/drawingapi.h>

#include <assert.h>

VertexFormat* VertexPos::format;
VertexFormat* VertexPosNorm::format;
VertexFormat* VertexPosUVNorm::format;
VertexFormat* VertexPosUV::format;

VertexFormat::VertexFormat(UINT BinaryFormat)
{
	memset(mAttributesArray, 0, sizeof(void*) * (UINT)VertexAttributeUsage::COUNT);

	this->mBinaryFormat = BinaryFormat;
	int stride = 0;
	for (int i=0; BinaryFormat; BinaryFormat>>=1, i++)
	{
		if (BinaryFormat & 1)
		{
			VertexAttribute attrib;
			attrib.Usage = (VertexAttributeUsage)i;
			attrib.Size = gVariableByteSizes[(UINT)gVertexAttributeType[i]];
			attrib.Offset = stride;
			mAttributes.push_back(attrib);
			stride += attrib.Size;
		}
	}

	for (UINT i=0; i<mAttributes.size(); i++)
	{
		VertexAttribute& attrib = mAttributes[i];

		/// AttributesArray points into the vector. Meh.
		mAttributesArray[(UINT)attrib.Usage] = &attrib;
	}

	mStride = stride;
	//Declaration = TheDrawingAPI->CreateVertexDeclaration(Attributes);

	assert(stride % 4 == 0);	/// Vertex structure size should always be mod4.
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
	return (mBinaryFormat & (1 << (UINT)Attrib)) != 0;
}

Mesh::Mesh()
{
	mVertexCount = 0;
	mVertexBufferSize = 0;
	mVertexHandle = 0;

	mIndexCount = 0;
	mIndexHandle = 0;

	mWireframeIndexCount = 0;
	mWireframeIndexHandle = 0;
	
	mFormat = NULL;
}

Mesh::~Mesh()
{
	if (mVertexHandle) TheDrawingAPI->DestroyVertexBuffer(mVertexHandle);
	if (mIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mIndexHandle);
	if (mWireframeIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mWireframeIndexHandle);
}

void Mesh::AllocateVertices(VertexFormat* Format, UINT VertexCount)							
{
	this->mFormat = Format;
	this->mVertexCount = VertexCount;

	UINT newBufferSize = Format->mStride * VertexCount;
	if (mVertexBufferSize != newBufferSize)
	{
		mVertexBufferSize = newBufferSize;

		if (mVertexHandle) TheDrawingAPI->DestroyVertexBuffer(mVertexHandle);
		mVertexHandle = TheDrawingAPI->CreateVertexBuffer(newBufferSize);
	}
}

void Mesh::AllocateIndices(UINT IndexCount)
{
	if (this->mIndexCount != IndexCount)
	{
		this->mIndexCount = IndexCount;
		if (mIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mIndexHandle);
		mIndexHandle = IndexCount ? TheDrawingAPI->CreateIndexBuffer(IndexCount * sizeof(IndexEntry)) : NULL;
	}
}

void Mesh::AllocateWireframeIndices(UINT IndexCount)
{
	if (mWireframeIndexCount != IndexCount)
	{
		mWireframeIndexCount = IndexCount;
		if (mWireframeIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mWireframeIndexHandle);
		mWireframeIndexHandle = IndexCount ? TheDrawingAPI->CreateIndexBuffer(mWireframeIndexCount) : NULL;
	}
}

void Mesh::UploadIndices(const IndexEntry* Indices)
{
	//TheDrawingAPI->UploadIndices(IndexHandle, IndexCount, Indices);
	void* mappedIndices = TheDrawingAPI->MapIndexBuffer(mIndexHandle);
	memcpy(mappedIndices, Indices, mIndexCount * sizeof(IndexEntry));
	TheDrawingAPI->UnMapIndexBuffer(mIndexHandle);
}

void Mesh::UploadVertices(void* Vertices)
{
	void* mappedMesh = TheDrawingAPI->MapVertexBuffer(mVertexHandle);
	memcpy(mappedMesh, Vertices, mVertexCount * mFormat->mStride);
	TheDrawingAPI->UnMapVertexBuffer(mVertexHandle);
}


void Mesh::UploadVertices( void* Vertices, int VertexCount )
{
	void* mappedMesh = TheDrawingAPI->MapVertexBuffer(mVertexHandle);
	memcpy(mappedMesh, Vertices, VertexCount * mFormat->mStride);
	TheDrawingAPI->UnMapVertexBuffer(mVertexHandle);
}

//const wchar_t* VertexAttribute::GetAttribString( VertexAttributeType Type )
//{
//	return Framework::GetWStringFromEnum(AttributeTypeMapper,  Type);
//}