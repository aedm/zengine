#include "document.h"


Document::Document()
	: Node(NodeType::DOCUMENT, "Document")
	, Graphs(NodeType::GRAPH, this, nullptr, true)
{

}

GraphNode::GraphNode()
	: Node(NodeType::GRAPH, "Graph")
	, Widgets(NodeType::WIDGET, this, nullptr, true)
{

}
