#include <include/dom/document.h>

REGISTER_NODECLASS(Document, "Document");

Document::Document()
  : mGraphs(this, "graphs", true)
  , mMovie(this, "movie")
  , mProperties(this, "properties", false, true)
{}
