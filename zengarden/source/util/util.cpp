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

    StubNode* vertexStub = new StubNode();
    vertexStub->SetStubSource(string(vertexContent.get()));
    StubNode* fragmentStub = new StubNode();
    fragmentStub->SetStubSource(string(fragmentContent.get()));

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

} // namespace Util
