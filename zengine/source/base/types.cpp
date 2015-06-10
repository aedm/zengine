#include <include/base/types.h>

/// Array for variable sizes in bytes
const char* gVariableNames[] = {
#undef ITEM
#define ITEM(name, type, token) token,
  VALUETYPE_LIST
};


/// Array for variable sizes in bytes
const int gVariableByteSizes[] = {
#undef ITEM
#define ITEM(name, type, token) sizeof(type),
  VALUETYPE_LIST
};


/// Array for attribute types
const NodeType gVertexAttributeType[] = {
#undef ITEM
#define ITEM(name, type, token) type,
  VERTEXATTRIBUTE_LIST
};


/// Array for attribute names
const char* gVertexAttributeName[] = {
#undef ITEM
#define ITEM(name, type, token) token,
  VERTEXATTRIBUTE_LIST
};
