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
  AddPrototype(new FloatNode(), NodeClass::STATIC_FLOAT);
  AddPrototype(new Vec4Node(), NodeClass::STATIC_VEC4);
  AddPrototype(new TextureNode(), NodeClass::STATIC_TEXTURE);
  AddPrototype(new Pass(), NodeClass::PASS);
}

void Prototypes::AddPrototype(Node* node, NodeClass nodeClass) {
  mPrototypeNodes.push_back(node);
  mNodeIndexMap[type_index(typeid(*node))] = nodeClass;
}

void Prototypes::AddStub(ShaderStub* stub) {
  mPrototypeNodes.push_back(stub);
}

Prototypes::~Prototypes() {
  for (Node* node : mPrototypeNodes) {
    delete node;
  }
  mPrototypeNodes.clear();
}

Node* Prototypes::AskUser(QWidget* Parent, QPoint Position) {
  QDialog dialog(Parent, Qt::FramelessWindowHint);
  mDialog = &dialog;
  Ui::OperatorSelector selector;
  selector.setupUi(&dialog);
  for (int i = 0; i < mPrototypeNodes.size(); i++) {
    Node* node = mPrototypeNodes[i];
    QString name;
    if (node->GetType() == NodeType::SHADER_STUB) {
      name = QString::fromStdString(
        static_cast<ShaderStub*>(node)->GetStubMetadata()->name);
    } else {
      name = GetNodeClassString(node);
    }
    selector.treeWidget->addTopLevelItem(new SelectorItem(NULL, name, i + 1));
  }
  dialog.setModal(true);
  dialog.resize(150, 300);
  dialog.move(Position);
  connect(selector.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)),
          this, SLOT(HandleItemSelected(QTreeWidgetItem*, int)));
  //dialog.connect(SIGNAL(itemClicked()), this, SLOT(OnItemSelected()));
  int ret = dialog.exec();
  return (ret > 0) ? mPrototypeNodes[ret - 1]->Clone() : NULL;
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

QString Prototypes::GetNodeClassString(Node* Nd) {
  switch (GetNodeClass(Nd)) {
    case NodeClass::STATIC_FLOAT:		return QString("Static Float");
    case NodeClass::STATIC_TEXTURE:		return QString("Static Texture");
    case NodeClass::STATIC_VEC4:		return QString("Static Vec4");
    case NodeClass::SHADER_STUB:		return QString("ShaderStub");
    case NodeClass::SHADER_SOURCE:		return QString("Shader Source");
    case NodeClass::PASS:				return QString("Pass");
    case NodeClass::UNKNOWN:			return QString("unknown");
  }
  ASSERT(false);
  return QString();
}

NodeClass Prototypes::GetNodeClass(Node* Nd) {
  try {
    const type_info& tid = typeid(*Nd);
    auto tin = type_index(tid);
    return mNodeIndexMap.at(type_index(typeid(*Nd)));
  } catch (out_of_range ex) {
    return NodeClass::UNKNOWN;
  }
}
