#include <include/dom/graph.h>

Graph::Graph()
  : Node(NodeType::GRAPH)
  , mNodes(NodeType::ALLOW_ALL, this, nullptr, true) 
{

}
