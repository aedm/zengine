#include <include/dom/document.h>

REGISTER_NODECLASS(Document);

static SharedString GraphSlotName = make_shared<string>("graphs");

Document::Document()
  : Node(NodeType::DOCUMENT)
  , mGraphs(NodeType::GRAPH, this, GraphSlotName, true) 
{

}
