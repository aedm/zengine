#pragma once

#include "graph/nodewidget.h"
#include <vector>

using namespace std;

class NodeGraph;


class Document
{
public:
	Document();

	vector<NodeGraph*>		Graphs;
};


class NodeGraph
{
public:
	NodeGraph();

	vector<NodeWidget*>		Widgets;
};
