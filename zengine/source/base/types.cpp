#include <include/base/types.h>


/// Array for attribute names
const char* gVertexAttributeName[] = {
#undef ITEM
#define ITEM(name, type, token) token,
  VERTEXATTRIBUTE_LIST
};
