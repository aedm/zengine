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
#include <set>

using namespace std;

extern const EnumMapperA TexelTypeMapper[];
extern const EnumMapperA SplineLayerMapper[];

class JSONSerializer {
public:
  JSONSerializer(const shared_ptr<Node>& root);
  string GetJSON();

private:
  /// Collect nodes in the transitive close of root
  void Traverse(const shared_ptr<Node>& root);

  /// Creates a JSON document from all the nodes
  void DumpNodes();

  /// Creates the JSON value for a Node
  rapidjson::Value Serialize(const shared_ptr<Node>& node);

  /// Node serializers
  void SerializeFloatNode(
    rapidjson::Value& nodeValue, const shared_ptr<FloatNode>& node);
  void SerializeVec2Node(
    rapidjson::Value& nodeValue, const shared_ptr<Vec2Node>& node);
  void SerializeVec3Node(
    rapidjson::Value& nodeValue, const shared_ptr<Vec3Node>& node);
  void SerializeVec4Node(
    rapidjson::Value& nodeValue, const shared_ptr<Vec4Node>& node);
  void SerializeFloatSplineNode(
    rapidjson::Value& nodeValue, const shared_ptr<FloatSplineNode>& node);
  void SerializeTextureNode(
    rapidjson::Value& nodeValue, const shared_ptr<TextureNode>& node);
  void SerializeStubNode(
    rapidjson::Value& nodeValue, const shared_ptr<StubNode>& node);
  void SerializeStaticMeshNode(
    rapidjson::Value& nodeValue, const shared_ptr<StaticMeshNode>& node);
  void SerializeGeneralNode(
    rapidjson::Value& nodeValue, const shared_ptr<Node>& node);

  /// Helpers
  rapidjson::Value SerializeVec2(const Vec2& vec);
  rapidjson::Value SerializeVec3(const Vec3& vec);
  rapidjson::Value SerializeVec4(const Vec4& vec);

  /// Number of nodes to save
  int mNodeCount;

  /// All nodes to save
  unordered_map<shared_ptr<Node>, int> mNodes;
  vector<shared_ptr<Node>> mNodesList;

  rapidjson::Document mJsonDocument;
  rapidjson::Document::AllocatorType* mAllocator;
};