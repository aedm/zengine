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

    /// Cleanup if unsuccesful
    if (!pass->isComplete()) {
      ERR("Pass incomplete.");
      delete pass;
      delete vertexStub;
      delete fragmentStub;
      return nullptr;
    }

    return pass;
  }

} // namespace Util
