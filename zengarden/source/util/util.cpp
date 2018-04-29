#include "util.h"
#include <QtCore/QDir>
#include <QMessageBox>
#include <memory>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "../commands/graphcommands.h"

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


  shared_ptr<Pass> LoadShader(const char* VertexFile, const char* FragmentFile) {
    unique_ptr<char> vertexContent(ReadFileQt(VertexFile));
    unique_ptr<char> fragmentContent(ReadFileQt(FragmentFile));

    if (!vertexContent || !fragmentContent) {
      ERR("Missing content.");
      return nullptr;
    }

    /// TODO: use LoadStub instead
    shared_ptr<StubNode> vertexStub = make_shared<StubNode>();
    vertexStub->mSource.SetDefaultValue(string(vertexContent.get()));
    shared_ptr<StubNode> fragmentStub = make_shared<StubNode>();
    fragmentStub->mSource.SetDefaultValue(string(fragmentContent.get()));

    shared_ptr<Pass> pass = make_shared<Pass>();
    pass->mVertexStub.Connect(vertexStub);
    pass->mFragmentStub.Connect(fragmentStub);

    return pass;
  }

  static Vec3 ObjLineToVec3(const QString& line) {
    QStringList v = line.split(' ', QString::SkipEmptyParts);
    if (v.size() != 4) {
      ERR("Syntax error in .obj line: %s", line.toLatin1());
      return Vec3(0, 0, 0);
    }
    return Vec3(v[1].toFloat(), v[2].toFloat(), v[3].toFloat());
  }

  OWNERSHIP Mesh* LoadMesh2(const QString& fileName) {
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
      } else if (line.startsWith("vn ")) {
        /// Normal definition
        normals.push_back(ObjLineToVec3(line));
      } else if (line.startsWith("vt ")) {
        /// Texcoord definition
        Vec3 t = ObjLineToVec3(line);
        texcoord.push_back(Vec2(t.x, t.y));
      } else if (line.startsWith("f ")) {
        /// Face definition
        QStringList t = line.right(line.length() - 2).split(' ', QString::SkipEmptyParts);
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
      vertices[i].uv = tindex == 0 ? Vec2(0, 0) : texcoord[tindex - 1];
    }

    Mesh* mesh = TheResourceManager->CreateMesh();
    mesh->AllocateVertices(VertexPosUVNorm::format, triplets.size());
    mesh->UploadVertices(&vertices[0]);

    return mesh;
  }

  static Vec3 ToVec3(const aiVector3D& v) {
    return Vec3(v.x, v.y, v.z);
  }

  OWNERSHIP Mesh* LoadMesh(const QString& fileName) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(fileName.toStdString(),
                                             aiProcess_CalcTangentSpace |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType |
                                             aiProcess_FlipWindingOrder |
                                             aiProcess_GenSmoothNormals);
    if (!scene) {
      ERR(importer.GetErrorString());
      return nullptr;
    }
    if (!scene->HasMeshes()) {
      ERR("File has no meshes");
      return nullptr;
    }
    if (scene->mNumMeshes > 1) {
      WARN("File has more than one mesh, importing first one.");
    }

    const aiMesh* mesh = scene->mMeshes[0];
    if (!mesh->HasFaces()) {
      ERR("Mesh has no faces.");
      return nullptr;
    }
    if (!mesh->HasNormals()) {
      ERR("Mesh has no normals.");
      return nullptr;
    }
    if (!mesh->HasTangentsAndBitangents()) {
      ERR("Mesh has no tangents.");
      return nullptr;
    }
    if (mesh->GetNumUVChannels() == 0) {
      ERR("Mesh has no UV.");
      return nullptr;
    }

    INFO("Importing mesh: %d faces, %d vertices", mesh->mNumFaces, mesh->mNumVertices);

    vector<IndexEntry> indices;
    for (UINT i = 0; i < mesh->mNumFaces; i++) {
      aiFace& face = mesh->mFaces[i];
      if (face.mNumIndices != 3) {
        ERR("Face #%d is not triangulated.", i);
        return nullptr;
      }
      for (UINT o = 0; o < 3; o++) {
        indices.push_back(face.mIndices[o]);
      }
    }

    vector<VertexPosUVNormTangent> vertices(mesh->mNumVertices);
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
      vertices[i].position = ToVec3(mesh->mVertices[i]);
      Vec3 uv = ToVec3(mesh->mTextureCoords[0][i]);
      vertices[i].uv = Vec2(uv.x, uv.y);
      vertices[i].normal = ToVec3(mesh->mNormals[i]);
      vertices[i].tangent = ToVec3(mesh->mTangents[i]);
    }

    Mesh* zenmesh = TheResourceManager->CreateMesh();
    zenmesh->AllocateVertices(VertexPosUVNormTangent::format, vertices.size());
    zenmesh->UploadVertices(&vertices[0]);
    zenmesh->AllocateIndices(indices.size());
    zenmesh->UploadIndices(&indices[0]);

    return zenmesh;
  }

  shared_ptr<StubNode> LoadStub(const QString& fileName) {
    unique_ptr<char> stubSource(Util::ReadFileQt(fileName));
    /// TODO: register this as an engine node
    shared_ptr<StubNode> stub = make_shared<StubNode>();
    stub->mSource.SetDefaultValue(stubSource.get());
    return stub;
  }

  void DisposeNodes(const set<shared_ptr<Node>>& nodes)
  {
    vector<shared_ptr<Node>> nodeList(nodes.begin(), nodes.end());
    for (UINT i = 0; i < nodeList.size(); i++) {
      shared_ptr<Node> node = nodeList[i];
      for (Slot* slot : node->GetDependants()) {
        shared_ptr<Node> refNode = slot->GetOwner();
        if (!refNode->IsGhostNode()) continue;
        if (std::find(nodeList.begin(), nodeList.end(), refNode) == nodeList.end()) {
          /// Ghost node wasn't in the list, add it
          nodeList.push_back(refNode);
        }
      }
    }
    if (nodeList.size() > nodes.size()) {
      auto button = QMessageBox::question(nullptr, "Delete nodes",
        "Ghost nodes found. Do you really want to delete?");
      if (button == QMessageBox::NoButton) return;
    }
    TheCommandStack->Execute(new DisposeNodesCommand(nodeList));
  }

} // namespace Util
