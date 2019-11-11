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

class JSONDeserializer {
public:
  JSONDeserializer(const std::string& json);
  std::shared_ptr<Document> GetDocument() const;

private:
  void DeserializeNode(rapidjson::Value& value);

  static void DeserializeFloatNode(const rapidjson::Value& value, 
    const std::shared_ptr<FloatNode>& node);
  static void DeserializeVec2Node(const rapidjson::Value& value, 
    const std::shared_ptr<Vec2Node>& node);
  static void DeserializeVec3Node(const rapidjson::Value& value, 
    const std::shared_ptr<Vec3Node>& node);
  static void DeserializeVec4Node(const rapidjson::Value& value, 
    const std::shared_ptr<Vec4Node>& node);
  static void DeserializeFloatSplineNode(const rapidjson::Value& value, 
    const std::shared_ptr<FloatSplineNode>& node);

  void DeserializeStaticTextureNode(const rapidjson::Value& value, 
    const std::shared_ptr<StaticTextureNode>& node) const;
  static void DeserializeStubNode(const rapidjson::Value& value, 
    const std::shared_ptr<StubNode>& node);
  void DeserializeStaticMeshNode(const rapidjson::Value& value, 
    const std::shared_ptr<StaticMeshNode>& node) const;

  void ConnectSlots(rapidjson::Value& value);
  
  /// Connects a slot by "id" tag.
  void ConnectValueSlotById(const rapidjson::Value& value, Slot* slot);

  static vec2 DeserializeVec2(const rapidjson::Value& value);
  static vec3 DeserializeVec3(const rapidjson::Value& value);
  static vec4 DeserializeVec4(const rapidjson::Value& value);

  std::unordered_map<int, std::shared_ptr<Node>> mNodes;

  std::shared_ptr<Document> mDocument;
};