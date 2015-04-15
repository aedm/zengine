#pragma once

#include "graph/nodewidget.h"
#include <vector>

using namespace std;

class OperatorGraph;


class Document
{
public:
	Document();

	vector<OperatorGraph*>		Graphs;
};


class OperatorGraph
{
public:
	OperatorGraph();

	vector<NodeWidget*>		Widgets;
};
