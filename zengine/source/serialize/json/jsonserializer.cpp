#include "jsonserializer.h"
#include <include/base/helpers.h>
#include <rapidjson/include/rapidjson/writer.h>
#include <rapidjson/include/rapidjson/stringbuffer.h>

JSONSerializer::JSONSerializer(Node* root) {
  
}

string JSONSerializer::GetJSON() {
  return string("alpakka");
}

rapidjson::Value* JSONSerializer::Serialize(Node* node) {
  return nullptr;
}

