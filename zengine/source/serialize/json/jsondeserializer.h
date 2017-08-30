#pragma once

#include <include/dom/document.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/meshnode.h>
#include <include/nodes/splinenode.h>
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
  
  void DeserializeFloatNode(const rapidjson::Value& value, FloatNode* node);
  void DeserializeVec2Node(const rapidjson::Value& value, Vec2Node* node);
  void DeserializeVec3Node(const rapidjson::Value& value, Vec3Node* node);
  void DeserializeVec4Node(const rapidjson::Value& value, Vec4Node* node);
  void DeserializeFloatSplineNode(const rapidjson::Value& value, FloatSplineNode* node);

  void DeserializeTextureNode(const rapidjson::Value& value, TextureNode* node);
  void DeserializeStubNode(const rapidjson::Value& value, StubNode* node);
  void DeserializeStaticMeshNode(const rapidjson::Value& value, StaticMeshNode* node);

  void ConnectSlots(rapidjson::Value& value);
  
  /// Connects a slot by "id" tag.
  void ConnectValueSlotById(const rapidjson::Value& value, Slot* slot);
  
  Vec2 DeserializeVec2(const rapidjson::Value& value);
  Vec3 DeserializeVec3(const rapidjson::Value& value);
  Vec4 DeserializeVec4(const rapidjson::Value& value);

  unordered_map<int, Node*> mNodes;
  int mNodeCount = 0;

  Document* mDocument = nullptr;
};