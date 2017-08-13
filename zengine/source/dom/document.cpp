#include <include/dom/document.h>

REGISTER_NODECLASS(Document, "Document");

static SharedString GraphSlotName = make_shared<string>("graphs");
static SharedString MovieSlotName = make_shared<string>("movie");
static SharedString BPMSlotName = make_shared<string>("BPM");

Document::Document()
  : Node(NodeType::DOCUMENT)
  , mGraphs(this, GraphSlotName, true)
  , mMovie(this, MovieSlotName)
  , mBPM(this, BPMSlotName, false, true, true, 60, 160)
{
  mBPM.SetDefaultValue(128);
}
