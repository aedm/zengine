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

/// Unique identifier for all possible node types
enum class NodeClass 
{
	STATIC_FLOAT,
	STATIC_VEC4,
	STATIC_TEXTURE,
	SHADER_STUB,
	SHADER_SOURCE,
	PASS,

	UNKNOWN,
};

class Prototypes: public QObject
{ 
	Q_OBJECT

public:
	Prototypes();
	~Prototypes();

	static void					Init();
	static void					Dispose();

	Node*						AskUser(QWidget* Parent, QPoint Position);

	QString						GetNodeClassString(Node* Nd);
	NodeClass					GetNodeClass(Node* Nd);

	void						AddPrototype(Node* node, NodeClass nodeClass);

private:
	vector<Node*>				PrototypeNodes;
	unordered_map<type_index, NodeClass> NodeIndexMap;

	QDialog*					Dialog;

private slots:
	void						OnItemSelected(QTreeWidgetItem* Item, int Column);
};