#include <include/dom/graph.h>

REGISTER_NODECLASS(Graph, "Graph");

static SharedString NodesSlotName = make_shared<string>("nodes");

Graph::Graph()
  : Node(NodeType::GRAPH)
  , mNodes(NodeType::ALLOW_ALL, this, NodesSlotName, true)
{
}

void Graph::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
      NotifyWatchers(&Watcher::OnSlotConnectionChanged, message->mSlot);
      break;
    default: break;
  }
}
