#pragma once

#include <zengine.h>
#include <QDialog>
#include <QTreeWidgetItem>

class OperatorPrototypes;

extern OperatorPrototypes* ThePrototypes;

class OperatorPrototypes: public QObject
{ 
	Q_OBJECT

public:
	OperatorPrototypes();
	~OperatorPrototypes();

	static void					Init();
	static void					Dispose();

	Node*						AskUser(QPoint Position);
		
private:
	vector<Node*>				Prototypes;
	QDialog*					Dialog;

private slots:
	void						OnItemSelected(QTreeWidgetItem* Item, int Column);
};