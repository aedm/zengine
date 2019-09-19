#pragma once

#include <zengine.h>
#include <QDialog>
#include <QTreeWidgetItem>
#include <unordered_map>
#include <typeindex>
#include <QtCore/QString>

class Prototypes;
extern Prototypes* ThePrototypes;

using namespace std;

class Prototypes: public QObject {
  Q_OBJECT

public:
  Prototypes();
  ~Prototypes();

  static void Init();
  static void	Dispose();

  shared_ptr<Node> AskUser(QWidget* parent, QPoint position);

  void AddPrototype(NodeClass* nodeClass);
  //void AddStub(OWNERSHIP StubNode* stub);

  /// Load default engine stubs
  void LoadStubs();

private:
  /// A single prototype node
  struct Prototype {
    NodeClass* mNodeClass;
    shared_ptr<Node> mNode;
    QString mName;
  };

  /// Tree of prototypes
  struct Category {
    ~Category();
    QString mName;
    vector<Prototype*> mPrototypes;
    vector<Category*> mSubCategories;
  };

  Category mMainCategory;

  /// An item in the tree view
  class SelectorItem: public QTreeWidgetItem {
  public:
    SelectorItem(SelectorItem* Parent, QString Label, int prototypeIndex);
    int mPrototypeIndex;
  };
  
  /// Dialog window displaying the prototype selector
  QDialog* mDialog;

  /// Loads a stub folder
  void LoadStubFolder(QString folder, Category* category);

  /// Adds a category to the Qt tree widget
  void AddCategoryToTreeWidget(Category* category, QTreeWidget* treeWidget,
                               QTreeWidgetItem* parentItem, 
                               vector<const Prototype*>& allPrototypes);

  private slots:
  void HandleItemSelected(QTreeWidgetItem* Item, int Column) const;
};