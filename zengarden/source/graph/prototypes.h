#pragma once

#include <zengine.h>
#include <QDialog>
#include <QTreeWidgetItem>
#include <unordered_map>
#include <typeindex>

class Prototypes;
extern Prototypes* ThePrototypes;

using namespace std;

/// Unique identifier for all possible node types
enum class NodeClass 
{
	STATIC_FLOAT,
	STATIC_VEC4,
	STATIC_TEXTURE
};

class Prototypes: public QObject
{ 
	Q_OBJECT

public:
	Prototypes();
	~Prototypes();

	static void					Init();
	static void					Dispose();

	Node*						AskUser(QPoint Position);
		
private:
	void						AddPrototype(Node* node, NodeClass nodeClass);
	
	vector<Node*>				PrototypeNodes;
	unordered_map<type_index, NodeClass> NodeIndexMap;

	QDialog*					Dialog;

private slots:
	void						OnItemSelected(QTreeWidgetItem* Item, int Column);
};