#include "prototypes.h"
#include <ui_operatorSelector.h>
#include <QtCore/QDir>
#include "../util/util.h"

Prototypes* ThePrototypes = NULL;

Prototypes::SelectorItem::SelectorItem(SelectorItem* Parent, QString Label, 
                                       int prototypeIndex)
  : QTreeWidgetItem(Parent)
  , mPrototypeIndex(prototypeIndex) 
{
  setText(0, Label);
}


Prototypes::Prototypes() {
  NodeRegistry* registry = NodeRegistry::GetInstance();
  AddPrototype(registry->GetNodeClass<FloatNode>());
  AddPrototype(registry->GetNodeClass<Vec4Node>());
  AddPrototype(registry->GetNodeClass<Pass>());
  AddPrototype(registry->GetNodeClass<TimeNode>());

  LoadStubs();
}

void Prototypes::AddPrototype(NodeClass* nodeClass) {
  Prototype* prototype = new Prototype();
  prototype->mNodeClass = nodeClass;
  prototype->mNode = nullptr;
  prototype->mName = QString::fromStdString(nodeClass->mClassName);
  mMainCategory.mPrototypes.push_back(prototype);
}

//void Prototypes::AddStub(OWNERSHIP StubNode* stub) {
//  NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(stub);
//  Prototype prototype;
//  prototype.mNodeClass = nodeClass;
//  prototype.mNode = stub;
//  prototype.mName = stub->GetStubMetadata() == nullptr 
//    ? nodeClass->mClassName : stub->GetStubMetadata()->name;
//  mMainCategory.mPrototypes.push_back(prototype);
//}

Prototypes::~Prototypes() {
}

Node* Prototypes::AskUser(QWidget* Parent, QPoint Position) {
  QDialog dialog(Parent, Qt::FramelessWindowHint);
  mDialog = &dialog;
  Ui::OperatorSelector selector;
  selector.setupUi(&dialog);

  vector<const Prototype*> allPrototypes;
  AddCategoryToTreeWidget(&mMainCategory, selector.treeWidget, nullptr, allPrototypes);

  dialog.setModal(true);
  dialog.resize(200, 400);
  dialog.move(Position);
  connect(selector.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
          this, SLOT(HandleItemSelected(QTreeWidgetItem*, int)));
  //dialog.connect(SIGNAL(itemClicked()), this, SLOT(OnItemSelected()));

  int ret = dialog.exec();
  if (ret <= 0) return nullptr;
  const Prototype* prototype = allPrototypes[ret - 1];

  /// Create new node, possibly based on original
  if (prototype->mNode == nullptr) {
    return prototype->mNodeClass->Manufacture();
  }
  return prototype->mNodeClass->Manufacture(prototype->mNode);
}


void Prototypes::HandleItemSelected(QTreeWidgetItem* Item, int) {
  SelectorItem* item = static_cast<SelectorItem*>(Item);
  if (item->mPrototypeIndex >= 0) mDialog->done(item->mPrototypeIndex);
}

void Prototypes::Init() {
  ThePrototypes = new Prototypes();
}

void Prototypes::Dispose() {
  SafeDelete(ThePrototypes);
}

void Prototypes::LoadStubs() {
  LoadStubFolder(QString("engine/stubs"), &mMainCategory);
}

void Prototypes::LoadStubFolder(QString folder, Category* category) {
  static const QString shaderSuffix("shader");

  QDir dir(folder);
  for (QFileInfo& fileInfo : dir.entryInfoList()) {
    if (fileInfo.isDir()) {
      if (fileInfo.fileName().startsWith(".")) continue;
      Category* subCategory = new Category();
      subCategory->mName = fileInfo.fileName();
      category->mSubCategories.push_back(subCategory);
      LoadStubFolder(fileInfo.filePath(), subCategory);
    }
    else if (fileInfo.completeSuffix() == shaderSuffix) {
      INFO("shader found: %s", fileInfo.baseName().toLatin1().data());
      char* stubSource = Util::ReadFileQt(fileInfo.absoluteFilePath());

      StubNode* stub = new StubNode();
      stub->mSource.SetDefaultValue(stubSource);

      NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass<StubNode>();
      Prototype* prototype = new Prototype();
      prototype->mNodeClass = nodeClass;
      prototype->mNode = stub;
      prototype->mName = QString::fromStdString(stub->GetStubMetadata() == nullptr 
        ? nodeClass->mClassName : stub->GetStubMetadata()->name);
      category->mPrototypes.push_back(prototype);
    }
  }
}

void Prototypes::AddCategoryToTreeWidget(Category* category, QTreeWidget* treeWidget, 
                                         QTreeWidgetItem* parentItem, 
                                         vector<const Prototype*>& allPrototypes) 
{
  for (const Prototype* prototype : category->mPrototypes) {
    allPrototypes.push_back(prototype);
    SelectorItem* item = 
      new SelectorItem(nullptr, prototype->mName, allPrototypes.size());
    if (parentItem == nullptr) treeWidget->addTopLevelItem(item);
    else parentItem->addChild(item);
  }

  for (Category* subCategory : category->mSubCategories) {
    SelectorItem* item = new SelectorItem(nullptr, subCategory->mName, -1);
    if (parentItem == nullptr) treeWidget->addTopLevelItem(item);
    else parentItem->addChild(item);
    AddCategoryToTreeWidget(subCategory, treeWidget, item, allPrototypes);
    item->setExpanded(true);
  }
}

Prototypes::Category::~Category() {
  for (Category* category : mSubCategories) {
    delete category;
  }
  for (Prototype* prototype : mPrototypes) {
    delete prototype;
  }
}

Prototypes::Prototype::~Prototype() {
  SafeDelete(mNode);
}
