#include "util.h"
#include <QtCore/QDir>
#include <QtWidgets/QMessageBox>
#include <memory>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "../commands/graphCommands.h"

namespace Util {

  OWNERSHIP char* ReadFileQt(const char* FileName) {
    return ReadFileQt(QString::fromLatin1(FileName));
  }

  /// TODO: return a string instead
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


  std::shared_ptr<Pass> LoadShader(const char* VertexFile, const char* FragmentFile) {
    const std::unique_ptr<char> vertexContent(ReadFileQt(VertexFile));
    const std::unique_ptr<char> fragmentContent(ReadFileQt(FragmentFile));

    if (!vertexContent || !fragmentContent) {
      ERR("Missing content.");
      return nullptr;
    }

    /// TODO: use LoadStub instead
    std::shared_ptr<StubNode> vertexStub = std::make_shared<StubNode>();
    vertexStub->mSource.SetDefaultValue(std::string(vertexContent.get()));
    std::shared_ptr<StubNode> fragmentStub = std::make_shared<StubNode>();
    fragmentStub->mSource.SetDefaultValue(std::string(fragmentContent.get()));

    std::shared_ptr<Pass> pass = std::make_shared<Pass>();
    pass->mVertexStub.Connect(vertexStub);
    pass->mFragmentStub.Connect(fragmentStub);

    return pass;
  }

  static vec3 ToVec3(const aiVector3D& v) {
    return vec3(v.x, v.y, v.z);
  }

  std::shared_ptr<Mesh> LoadMesh(const QString& fileName) {
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

    std::vector<IndexEntry> indices;
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

    std::vector<VertexPosUvNormTangent> vertices(mesh->mNumVertices);
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
      vertices[i].mPosition = ToVec3(mesh->mVertices[i]);
      const vec3 uv = ToVec3(mesh->mTextureCoords[0][i]);
      vertices[i].mUv = vec2(uv.x, uv.y);
      vertices[i].mNormal = ToVec3(mesh->mNormals[i]);
      vertices[i].mTangent = ToVec3(mesh->mTangents[i]);
    }

    std::shared_ptr<Mesh> zenmesh = std::make_shared<Mesh>();
    zenmesh->AllocateVertices(VertexPosUvNormTangent::mFormat, vertices.size());
    zenmesh->UploadVertices(&vertices[0]);
    zenmesh->AllocateIndices(indices.size());
    zenmesh->UploadIndices(&indices[0]);

    return zenmesh;
  }

  std::shared_ptr<StubNode> LoadStub(const QString& fileName) {
    const std::unique_ptr<char> stubSource(Util::ReadFileQt(fileName));
    /// TODO: register this as an engine node
    auto stub = std::make_shared<StubNode>();
    stub->mSource.SetDefaultValue(stubSource.get());
    return stub;
  }

  void DisposeNodes(const std::set<std::shared_ptr<Node>>& nodes)
  {
    std::vector<std::shared_ptr<Node>> nodeList(nodes.begin(), nodes.end());
    for (UINT i = 0; i < nodeList.size(); i++) {
      const std::shared_ptr<Node> node = nodeList[i];
      for (Slot* slot : node->GetDependants()) {
        std::shared_ptr<Node> refNode = slot->GetOwner();
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
