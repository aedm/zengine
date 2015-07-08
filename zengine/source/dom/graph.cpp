#include <include/dom/graph.h>

REGISTER_NODECLASS(Graph);

static SharedString NodesSlotName = make_shared<string>("nodes");

Graph::Graph()
  : Node(NodeType::GRAPH)
  , mNodes(NodeType::ALLOW_ALL, this, NodesSlotName, true)
{

}
