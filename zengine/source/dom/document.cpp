#include <include/dom/document.h>

REGISTER_NODECLASS(Document);

Document::Document()
  : Node(NodeType::DOCUMENT)
  , mGraphs(NodeType::GRAPH, this, nullptr, true) 
{

}
