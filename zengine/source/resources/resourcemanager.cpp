#include <include/resources/resourcemanager.h>
#include <boost/foreach.hpp>

ResourceManager::ResourceManager()
{
	VertexPos::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK);
	VertexPosNorm::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_NORMAL_MASK);
	VertexPosUVNorm::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_NORMAL_MASK | VERTEXATTRIB_TEXCOORD_MASK);
	VertexPosUV::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_TEXCOORD_MASK);
}

ResourceManager::~ResourceManager()
{
	//for (UINT i=0; i<AllTextures.size(); i++) delete AllTextures[i];
	//for (UINT i=0; i<AllShaderSources.size(); i++) delete AllShaderSources[i];
	//for (set<Mesh*>::iterator it = Meshes.begin(); it != Meshes.end(); it++) delete *it;

	//for (map<TexCategory, TextureList*>::iterator it = FreeTextures.begin(); it != FreeTextures.end(); it++)
	//{
	//	delete it->second;
	//}

	foreach (Mesh* mesh, mMeshes) delete mesh;
	foreach (auto vertexFormat, mVertexFormats) delete vertexFormat.second;
}

//Texture* ResourceManager::CreateTexture(int Width, int Height, TextureType Type, void* PixelData)
//{
//	TexCategory category(Width, Height, Type);
//	map<TexCategory, TextureList*>::iterator it = FreeTextures.find(category);
//
//	if (it != FreeTextures.end())
//	{
//		TextureList* textureList = it->second;
//		if (textureList->size() > 0)
//		{
//			Texture* texture = (*textureList)[textureList->size()-1];
//			textureList->pop_back();
//			return texture;
//		}
//	}
//
//	Texture* texture = new Texture(Width, Height, Type, PixelData);
//	AllTextures.push_back(texture);
//
//	return texture;
//}

//void ResourceManager::DiscardTexture( Texture* TextureInstance )
//{
//	if (TextureInstance)
//	{
//		TexCategory category(TextureInstance->Width, TextureInstance->Height, TextureInstance->Type);
//		map<TexCategory, TextureList*>::iterator it = FreeTextures.find(category);
//
//		if (it != FreeTextures.end())
//		{
//			it->second->push_back(TextureInstance);
//		}
//		else
//		{
//			TextureList* texList = new TextureList();
//			texList->push_back(TextureInstance);
//			FreeTextures[category] = texList;
//		}
//	}
//}

//ShaderSource* ResourceManager::CreateShaderSource( ShaderType Type, const char* Source )
//{
//	ShaderSource* shaderSource = new ShaderSource(Type, Source);
//	AllShaderSources.push_back(shaderSource);
//	return shaderSource;
//}

VertexFormat* ResourceManager::GetVertexFormat( UINT BinaryFormat )
{
	map<UINT, VertexFormat*>::iterator it = mVertexFormats.find(BinaryFormat);
	if (it != mVertexFormats.end()) return it->second;

	VertexFormat* format = new VertexFormat(BinaryFormat);
	mVertexFormats[BinaryFormat] = format;

	return format;
}

Mesh* ResourceManager::CreateMesh()
{
	Mesh* mesh = new Mesh();
	mMeshes.insert(mesh);
	return mesh;
}

void ResourceManager::DiscardMesh( Mesh* MeshInstance )
{
	mMeshes.erase(MeshInstance);
	delete MeshInstance;
}

Texture* ResourceManager::CreateTexture( int Width, int Height, TexelTypeEnum Type, void* TexelData )
{
	TextureHandle handle = TheDrawingAPI->CreateTexture(Width, Height, Type);
	if (TexelData) 
	{
		TheDrawingAPI->UploadTextureData(handle, Width, Height, Type, TexelData);
		// TODO: error handling
	}
	Texture* texture = new Texture(Width, Height, Type, handle);
	return texture;
}

void ResourceManager::DiscardTexture( Texture* TextureInstance )
{
	TheDrawingAPI->DeleteTexture(TextureInstance->mHandle);
	delete TextureInstance;
}



//ResourceManager::TexCategory::TexCategory( int Width, int Height, TextureType Type )
//{
//	this->Width = Width;
//	this->Height = Height;
//	this->Type = Type;
//}

//bool ResourceManager::TexCategory::operator<( const ResourceManager::TexCategory& Category ) const 
//{
//	return Width < Category.Width || Height < Category.Height || Type < Category.Type;
//}
