#pragma once

#include <zengine.h>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTreeWidgetItem>
#include <typeindex>
#include <QtCore/QString>

class Prototypes;
extern Prototypes* ThePrototypes;

class Prototypes: public QObject {
  Q_OBJECT

public:
  Prototypes();
  ~Prototypes();

  static void Init();
  static void	Dispose();

  std::shared_ptr<Node> AskUser(QWidget* parent, QPoint position);

  void AddPrototype(NodeClass* nodeClass);
  //void AddStub(OWNERSHIP StubNode* stub);

  /// Load default engine stubs
  void LoadStubs();

private:
  /// A single prototype node
  struct Prototype {
    NodeClass* mNodeClass = nullptr;
    std::shared_ptr<Node> mNode;
    QString mName;
  };

  /// Tree of prototypes
  struct Category {
    ~Category();
    QString mName;
    std::vector<Prototype*> mPrototypes;
    std::vector<Category*> mSubCategories;
  };

  Category mMainCategory;

  /// An item in the tree view
  class SelectorItem: public QTreeWidgetItem {
  public:
    SelectorItem(SelectorItem* parent, const QString& label, int prototypeIndex);
    int mPrototypeIndex;
  };
  
  /// Dialog window displaying the prototype selector
  QDialog* mDialog = nullptr;

  /// Loads a stub folder
  static void LoadStubFolder(const QString& folder, Category* category);

  /// Adds a category to the Qt tree widget
  static void AddCategoryToTreeWidget(Category* category, QTreeWidget* treeWidget,
                               QTreeWidgetItem* parentItem,
                               std::vector<const Prototype*>& allPrototypes);

  private slots:
  void HandleItemSelected(QTreeWidgetItem* Item, int Column) const;
};