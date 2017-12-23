#include <include/dom/document.h>

REGISTER_NODECLASS(Document, "Document");

static SharedString GraphSlotName = make_shared<string>("graphs");
static SharedString MovieSlotName = make_shared<string>("movie");
static SharedString PropertiesSlotName = make_shared<string>("properties");

Document::Document()
  : mGraphs(this, GraphSlotName, true)
  , mMovie(this, MovieSlotName)
  , mProperties(this, PropertiesSlotName, false, true)
{
}
