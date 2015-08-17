#pragma once

#include <include/dom/document.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/meshnode.h>
#include <include/shaders/stubnode.h>
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
  void DeserializeStubNode(const rapidjson::Value& value, StubNode* node);
  void DeserializeStaticMeshNode(const rapidjson::Value& value, StaticMeshNode* node);

  void ConnectSlots(rapidjson::Value& value);
  
  /// Connects a slot by "id" tag.
  void ConnectValueSlotById(const rapidjson::Value& value, Slot* slot);
  
  Vec2 DeserializeVec2(rapidjson::Value& value);

  unordered_map<int, Node*> mNodes;
  int mNodeCount = 0;

  Document* mDocument = nullptr;
};