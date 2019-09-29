#pragma once

#include <include/dom/document.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/meshnode.h>
#include <include/nodes/texturenode.h>
#include <include/nodes/splinenode.h>
#include <include/shaders/stubnode.h>
#include <rapidjson/include/rapidjson/document.h>
#include <string>
#include <unordered_map>

using namespace std;

class JSONDeserializer {
public:
  JSONDeserializer(const string& json);
  shared_ptr<Document> GetDocument() const;

private:
  void DeserializeNode(rapidjson::Value& value);

  static void DeserializeFloatNode(const rapidjson::Value& value, 
    const shared_ptr<FloatNode>& node);
  static void DeserializeVec2Node(const rapidjson::Value& value, 
    const shared_ptr<Vec2Node>& node);
  static void DeserializeVec3Node(const rapidjson::Value& value, 
    const shared_ptr<Vec3Node>& node);
  static void DeserializeVec4Node(const rapidjson::Value& value, 
    const shared_ptr<Vec4Node>& node);
  static void DeserializeFloatSplineNode(const rapidjson::Value& value, 
    const shared_ptr<FloatSplineNode>& node);

  void DeserializeStaticTextureNode(const rapidjson::Value& value, 
    const shared_ptr<StaticTextureNode>& node) const;
  static void DeserializeStubNode(const rapidjson::Value& value, 
    const shared_ptr<StubNode>& node);
  void DeserializeStaticMeshNode(const rapidjson::Value& value, 
    const shared_ptr<StaticMeshNode>& node) const;

  void ConnectSlots(rapidjson::Value& value);
  
  /// Connects a slot by "id" tag.
  void ConnectValueSlotById(const rapidjson::Value& value, Slot* slot);

  static Vec2 DeserializeVec2(const rapidjson::Value& value);
  static Vec3 DeserializeVec3(const rapidjson::Value& value);
  static Vec4 DeserializeVec4(const rapidjson::Value& value);

  unordered_map<int, shared_ptr<Node>> mNodes;

  shared_ptr<Document> mDocument;
};