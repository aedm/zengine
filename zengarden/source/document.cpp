#include "document.h"


Document::Document()
{

}

GraphNode::GraphNode()
	: Node(NodeType::GRAPH, "Graph")
	, Widgets(NodeType::WIDGET, this, nullptr, true)
{

}
