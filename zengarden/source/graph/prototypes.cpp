#include "prototypes.h"
#include <ui_operatorSelector.h>

Prototypes* ThePrototypes = NULL;

class SelectorItem: public QTreeWidgetItem {
public:
  SelectorItem(SelectorItem* Parent, QString Label, int OpIndex)
    : QTreeWidgetItem(Parent) {
    setText(0, Label);
    this->NodeIndex = OpIndex;
  }

  int NodeIndex;
};


Prototypes::Prototypes() {
  //AddPrototype(new FloatNode(), NodeClass::STATIC_FLOAT);
  //AddPrototype(new Vec4Node(), NodeClass::STATIC_VEC4);
  //AddPrototype(new TextureNode(), NodeClass::STATIC_TEXTURE);
  //AddPrototype(new Pass(), NodeClass::PASS);
  NodeRegistry* registry = NodeRegistry::GetInstance();
  AddPrototype(registry->GetNodeClass<FloatNode>());
  AddPrototype(registry->GetNodeClass<Vec4Node>());
  AddPrototype(registry->GetNodeClass<TextureNode>());
  AddPrototype(registry->GetNodeClass<Pass>());
  AddPrototype(registry->GetNodeClass<TimeNode>());
}

void Prototypes::AddPrototype(NodeClass* nodeClass) {
  Prototype prototype;
  prototype.mNodeClass = nodeClass;
  prototype.mNode = nullptr;
  prototype.mName = nodeClass->mClassName;
  mPrototypes.push_back(prototype);
}

void Prototypes::AddStub(OWNERSHIP StubNode* stub) {
  NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(stub);
  Prototype prototype;
  prototype.mNodeClass = nodeClass;
  prototype.mNode = stub;
  prototype.mName = stub->GetStubMetadata() == nullptr 
    ? nodeClass->mClassName : stub->GetStubMetadata()->name;
  mPrototypes.push_back(prototype);
}

Prototypes::~Prototypes() {
  for (Prototype& prototype: mPrototypes) {
    SafeDelete(prototype.mNode);
  }
}

Node* Prototypes::AskUser(QWidget* Parent, QPoint Position) {
  QDialog dialog(Parent, Qt::FramelessWindowHint);
  mDialog = &dialog;
  Ui::OperatorSelector selector;
  selector.setupUi(&dialog);
  for (int i = 0; i < mPrototypes.size(); i++) {
    Prototype& prototype = mPrototypes[i];
    QString name = QString::fromStdString(prototype.mName);
    selector.treeWidget->addTopLevelItem(new SelectorItem(nullptr, name, i + 1));
  }
  dialog.setModal(true);
  dialog.resize(150, 300);
  dialog.move(Position);
  connect(selector.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
          this, SLOT(HandleItemSelected(QTreeWidgetItem*, int)));
  //dialog.connect(SIGNAL(itemClicked()), this, SLOT(OnItemSelected()));

  int ret = dialog.exec();
  if (ret <= 0) return nullptr;

  Prototype& prototype = mPrototypes[ret - 1];
  if (prototype.mNode == nullptr) {
    return prototype.mNodeClass->Manufacture();
  }
  return prototype.mNodeClass->Manufacture(prototype.mNode);
}

void Prototypes::HandleItemSelected(QTreeWidgetItem* Item, int) {
  SelectorItem* item = static_cast<SelectorItem*>(Item);
  if (item->NodeIndex >= 0) mDialog->done(item->NodeIndex);
}

void Prototypes::Init() {
  ThePrototypes = new Prototypes();
}

void Prototypes::Dispose() {
  SafeDelete(ThePrototypes);
}

//QString Prototypes::GetNodeClassString(Node* Nd) {
//  switch (GetNodeClass(Nd)) {
//    case NodeClass::STATIC_FLOAT:		return QString("Static Float");
//    case NodeClass::STATIC_TEXTURE:		return QString("Static Texture");
//    case NodeClass::STATIC_VEC4:		return QString("Static Vec4");
//    case NodeClass::SHADER_STUB:		return QString("ShaderStub");
//    case NodeClass::SHADER_SOURCE:		return QString("Shader Source");
//    case NodeClass::PASS:				return QString("Pass");
//    case NodeClass::UNKNOWN:			return QString("unknown");
//  }
//  ASSERT(false);
//  return QString();
//}

