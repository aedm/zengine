#include "document.h"


Document::Document()
	: Node(NodeType::DOCUMENT)
	, Graphs(NodeType::GRAPH, this, nullptr, true)
{

}

GraphNode::GraphNode()
	: Node(NodeType::GRAPH)
	, Widgets(NodeType::WIDGET, this, nullptr, true)
{

}
