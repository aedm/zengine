#include "glPainter.h"
//#include "operatorPrototypes.h"
#include <QDir>

void InitCanvas();
void DisposeCanvas();

void InitEditorComponents()
{
	Q_INIT_RESOURCE(ui_shaders);

	InitCanvas();
	//ThePrototypes = new OperatorPrototypes();
}

void CloseEditorComponents()
{
	//SafeDelete(ThePrototypes);
	DisposeCanvas();
}

char* ReadFileQt( const char* FileName )
{
	QFile file(FileName);
	if(!file.open(QFile::ReadOnly)) 
	{
		ERR("Can't open resource: %s", FileName);
		return NULL;
	}
	QByteArray byteArray = file.readAll();
	int len = byteArray.length();
	char* content = new char[len + 1];
	memcpy(content, byteArray.data(), len);
	content[len] = 0;
	return content;
}

ShaderNode* LoadShader( const char* VertexFile, const char* FragmentFile )
{
	char* vertexContent = ReadFileQt(VertexFile);
	char* fragmentContent = ReadFileQt(FragmentFile);

	if (vertexContent && fragmentContent) 
	{
		shared_ptr<ShaderSource> source(ShaderSource::FromSource(vertexContent, fragmentContent));
		Shader* shader = ShaderSource::Build(source);
		if (shader)
		{
			ShaderNode* shop = new ShaderNode(shader);
			return shop;
		}

		/// Cleanup if unsuccesful
		delete shader;
	}
	return NULL;
}