#include "stubloader.h"
#include "../graph/prototypes.h"
#include "../util/util.h"
#include <zengine.h>
#include <QtCore/QDir>

void StubLoader::LoadStubs() {
  QDir dir("engine/stubs", "*.shader");
  for (QFileInfo& fileInfo : dir.entryInfoList()) {
    INFO("shader found: %s", fileInfo.baseName().toLatin1().data());
    char* stubSource = Util::ReadFileQt(fileInfo.absoluteFilePath());
    StubNode* stub = new StubNode();
    stub->mSource.SetDefaultValue(stubSource);
    ThePrototypes->AddStub(stub);
  }
}

StubLoader::StubLoader() {

}
