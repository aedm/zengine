#pragma once

#include "graph/nodewidget.h"
#include <vector>

using namespace std;

class GraphNode;



class Document: public Node
{
public:
	Document();

	Slot					Graphs;
};


class GraphNode: public Node
{
public:
	GraphNode();

	Slot					Widgets;
};
