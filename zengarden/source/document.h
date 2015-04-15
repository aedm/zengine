#pragma once

#include "graph/operatorWidget.h"
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

	vector<OperatorWidget*>		Widgets;
};
