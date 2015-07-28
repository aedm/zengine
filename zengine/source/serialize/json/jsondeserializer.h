#pragma once

#include <include/dom/document.h>
#include <include/nodes/valuenodes.h>
#include <rapidjson/include/rapidjson/document.h>
#include <string>
#include <unordered_map>

using namespace std;

class JSONDeserializer {
public:
  JSONDeserializer(const string& json);
  Document* GetDocument();

private:
  void DeserializeNode(rapidjson::Value& value);
  void DeserializeTextureNode(const rapidjson::Value& value, TextureNode* node);

  void ConnectSlots(rapidjson::Value& value);
  Vec2 DeserializeVec2(rapidjson::Value& value);

  unordered_map<int, Node*> mNodes;
  int mNodeCount = 0;

  Document* mDocument = nullptr;
};