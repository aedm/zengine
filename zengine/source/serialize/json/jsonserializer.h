#pragma once

#include <include/dom/document.h>
#include <include/dom/ghost.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/meshnode.h>
#include <include/nodes/texturenode.h>
#include <include/nodes/splinenode.h>
#include <include/shaders/stubnode.h>
#include <string>
#include <rapidjson/include/rapidjson/document.h>
#include <unordered_map>

extern const EnumMapperA TexelTypeMapper[];
//extern EnumMap<SplineLayer, const char*> SplineLayerMapper;
inline constexpr auto SplineLayerMapper = MakeEnumMapA<SplineLayer>({
  {"base", SplineLayer::BASE},
  {"noise", SplineLayer::NOISE},
  {"beat_spike_intensity", SplineLayer::BEAT_SPIKE_INTENSITY},
  {"beat_spike_frequency", SplineLayer::BEAT_SPIKE_FREQUENCY},
  {"beat_quantizer", SplineLayer::BEAT_QUANTIZER},
});


class JSONSerializer {
public:
  JSONSerializer(const std::shared_ptr<Node>& root);
  std::string GetJSON() const;

private:
  /// Collect nodes in the transitive close of root
  void Traverse(const std::shared_ptr<Node>& root);

  /// Creates a JSON document from all the nodes
  void DumpNodes();

  /// Creates the JSON value for a Node
  rapidjson::Value Serialize(const std::shared_ptr<Node>& node);

  /// Node serializers
  void SerializeFloatNode(
    rapidjson::Value& nodeValue, const std::shared_ptr<FloatNode>& node) const;
  void SerializeVec2Node(
    rapidjson::Value& nodeValue, const std::shared_ptr<Vec2Node>& node) const;
  void SerializeVec3Node(
    rapidjson::Value& nodeValue, const std::shared_ptr<Vec3Node>& node) const;
  void SerializeVec4Node(
    rapidjson::Value& nodeValue, const std::shared_ptr<Vec4Node>& node) const;
  void SerializeFloatSplineNode(
    rapidjson::Value& nodeValue, const std::shared_ptr<FloatSplineNode>& node) const;
  void SerializeTextureNode(
    rapidjson::Value& nodeValue, const std::shared_ptr<TextureNode>& node) const;
  void SerializeStubNode(
    rapidjson::Value& nodeValue, const std::shared_ptr<StubNode>& node) const;
  void SerializeStaticMeshNode(
    rapidjson::Value& nodeValue, const std::shared_ptr<StaticMeshNode>& node) const;
  void SerializeGeneralNode(
    rapidjson::Value& nodeValue, const std::shared_ptr<Node>& node);

  /// Helpers
  rapidjson::Value SerializeVec2(const Vec2& vec) const;
  rapidjson::Value SerializeVec3(const Vec3& vec) const;
  rapidjson::Value SerializeVec4(const Vec4& vec) const;

  /// Number of nodes to save
  int mNodeCount;

  /// All nodes to save
  std::unordered_map<std::shared_ptr<Node>, int> mNodes;
  std::vector<std::shared_ptr<Node>> mNodesList;

  rapidjson::Document mJsonDocument;
  rapidjson::Document::AllocatorType* mAllocator;
};