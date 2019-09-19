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
      return nullptr;
    }
    QByteArray byteArray = file.readAll();
    const int len = byteArray.length();
    char* content = new char[len + 1];
    memcpy(content, byteArray.data(), len);
    content[len] = 0;
    return content;
  }


  shared_ptr<Pass> LoadShader(const char* VertexFile, const char* FragmentFile) {
    const unique_ptr<char> vertexContent(ReadFileQt(VertexFile));
    const unique_ptr<char> fragmentContent(ReadFileQt(FragmentFile));

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

  static Vec3 ToVec3(const aiVector3D& v) {
    return Vec3(v.x, v.y, v.z);
  }

  shared_ptr<Mesh> LoadMesh(const QString& fileName) {
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
      const Vec3 uv = ToVec3(mesh->mTextureCoords[0][i]);
      vertices[i].uv = Vec2(uv.x, uv.y);
      vertices[i].normal = ToVec3(mesh->mNormals[i]);
      vertices[i].tangent = ToVec3(mesh->mTangents[i]);
    }

    shared_ptr<Mesh> zenmesh = make_shared<Mesh>();
    zenmesh->AllocateVertices(VertexPosUVNormTangent::format, vertices.size());
    zenmesh->UploadVertices(&vertices[0]);
    zenmesh->AllocateIndices(indices.size());
    zenmesh->UploadIndices(&indices[0]);

    return zenmesh;
  }

  shared_ptr<StubNode> LoadStub(const QString& fileName) {
    const unique_ptr<char> stubSource(Util::ReadFileQt(fileName));
    /// TODO: register this as an engine node
    shared_ptr<StubNode> stub = make_shared<StubNode>();
    stub->mSource.SetDefaultValue(stubSource.get());
    return stub;
  }

  void DisposeNodes(const set<shared_ptr<Node>>& nodes)
  {
    vector<shared_ptr<Node>> nodeList(nodes.begin(), nodes.end());
    for (UINT i = 0; i < nodeList.size(); i++) {
      const shared_ptr<Node> node = nodeList[i];
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
      const auto button = QMessageBox::question(nullptr, "Delete nodes",
        "Ghost nodes found. Do you really want to delete?");
      if (button == QMessageBox::NoButton) return;
    }
    TheCommandStack->Execute(new DisposeNodesCommand(nodeList));
  }

} // namespace Util
