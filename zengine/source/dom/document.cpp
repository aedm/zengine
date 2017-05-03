#include <include/dom/document.h>

REGISTER_NODECLASS(Document, "Document");

static SharedString GraphSlotName = make_shared<string>("graphs");
static SharedString MovieSlotName = make_shared<string>("movie");

Document::Document()
  : Node(NodeType::DOCUMENT)
  , mGraphs(this, GraphSlotName, true) 
  , mMovie(this, MovieSlotName)
{}
