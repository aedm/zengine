#include "util.h"
#include <QtCore/QDir>
#include <memory>

using namespace std;

namespace Util {

  OWNERSHIP char* ReadFileQt(const char* FileName) {
    return ReadFileQt(QString::fromLatin1(FileName));
  }

  OWNERSHIP char* ReadFileQt(const QString& FileName) {
    QFile file(FileName);
    if (!file.open(QFile::ReadOnly)) {
      ERR("Can't open file/resource: %s", FileName);
      return NULL;
    }
    QByteArray byteArray = file.readAll();
    int len = byteArray.length();
    char* content = new char[len + 1];
    memcpy(content, byteArray.data(), len);
    content[len] = 0;
    return content;
  }


  Pass* LoadShader(const char* VertexFile, const char* FragmentFile) {
    unique_ptr<char> vertexContent(ReadFileQt(VertexFile));
    unique_ptr<char> fragmentContent(ReadFileQt(FragmentFile));

    if (!vertexContent || !fragmentContent) {
      ERR("Missing content.");
      return nullptr;
    }

    /// TODO: use LoadStub instead
    StubNode* vertexStub = new StubNode();
    vertexStub->mSource.SetDefaultValue(string(vertexContent.get()));
    StubNode* fragmentStub = new StubNode();
    fragmentStub->mSource.SetDefaultValue(string(fragmentContent.get()));

    Pass* pass = new Pass();
    pass->mVertexStub.Connect(vertexStub);
    pass->mFragmentStub.Connect(fragmentStub);

    return pass;
  }



  class TopologicalOrder {
  public:
    TopologicalOrder(Node* root, vector<Node*>& oResult) {
      mResult = &oResult;
      Traverse(root);
    }

  private:
    void Traverse(Node* node) {
      if (mVisited.find(node) != mVisited.end()) return;
      mVisited.insert(node);

      for (Slot* slot : node->GetPublicSlots()) {
        if (slot->mIsMultiSlot) {
          for (Node* dependency : slot->GetMultiNodes()) {
            Traverse(dependency);
          }
        }
        else if (!slot->IsDefaulted()) {
          Node* dependency = slot->GetAbstractNode();
          if (dependency != nullptr) Traverse(dependency);
        }
      }

      mResult->push_back(node);
    }
    
    vector<Node*>* mResult;
    set<Node*> mVisited;
  };

  void CreateTopologicalOrder(Node* root, vector<Node*>& oResult) {
    TopologicalOrder tmp(root, oResult);
  }

  static Vec3 ObjLineToVec3(const QString& line) {
    QStringList v = line.split(' ', QString::SkipEmptyParts);
    if (v.size() != 4) {
      ERR("Syntax error in .obj line: %s", line.toLatin1());
      return Vec3(0, 0, 0);
    }
    return Vec3(v[1].toFloat(), v[2].toFloat(), v[3].toFloat());
  }
  
  OWNERSHIP Mesh* LoadMesh(const QString& fileName) {
    unique_ptr<char> fileContent(ReadFileQt(fileName));
    QString contentString(fileContent.get());
    QStringList lines = contentString.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    
    vector<Vec3> coords;
    vector<Vec3> normals;
    vector<Vec2> texcoord;
    struct Triplet { UINT c, n, t; };
    vector<Triplet> triplets;

    for (QString& line : lines) {
      if (line.startsWith("v ")) {
        /// Vertex definition
        coords.push_back(ObjLineToVec3(line));
      }
      else if (line.startsWith("vn ")) {
        /// Normal definition
        normals.push_back(ObjLineToVec3(line));
      }
      else if (line.startsWith("vt ")) {
        /// Texcoord definition
        Vec3 t = ObjLineToVec3(line);
        texcoord.push_back(Vec2(t.x, t.y));
      }
      else if (line.startsWith("f ")) {
        /// Face definition
        QStringList t = line.right(line.length()-2).split(' ', QString::SkipEmptyParts);
        for (QString& s : t) {
          QStringList a = s.split('/');
          triplets.push_back({a[0].toUInt(), a[2].toUInt(), a[1].toUInt()});
        }
      }
    }

    vector<VertexPosUVNorm> vertices(triplets.size());

    for (UINT i = 0; i < triplets.size(); i++) {
      vertices[i].position = coords[triplets[i].c - 1];
      vertices[i].normal = normals[triplets[i].n - 1];
      UINT tindex = triplets[i].t;
      vertices[i].uv = tindex == 0 ? Vec2(0, 0) : texcoord[tindex-1];
    }

    Mesh* mesh = TheResourceManager->CreateMesh();
    mesh->AllocateVertices(VertexPosUVNorm::format, triplets.size());
    mesh->UploadVertices(&vertices[0]);

    return mesh;
  }

  StubNode* LoadStub(const QString& fileName)
  {
    unique_ptr<char> stubSource(Util::ReadFileQt(fileName));
    /// TODO: register this as an engine node
    StubNode* stub = new StubNode();
    stub->mSource.SetDefaultValue(stubSource.get());
    return stub;
  }

} // namespace Util
