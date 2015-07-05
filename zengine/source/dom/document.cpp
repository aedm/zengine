#include <include/dom/document.h>

Document::Document()
  : Node(NodeType::DOCUMENT)
  , mGraphs(NodeType::GRAPH, this, nullptr, true) 
{

}
