#include <include/dom/graph.h>

REGISTER_NODECLASS(Graph);

Graph::Graph()
  : Node(NodeType::GRAPH)
  , mNodes(NodeType::ALLOW_ALL, this, nullptr, true) 
{

}
