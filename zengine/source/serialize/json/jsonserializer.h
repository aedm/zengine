#pragma once

#include <include/dom/document.h>
#include <include/nodes/valuenodes.h>
#include <include/nodes/meshnode.h>
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
  JSONSerializer(Node* root);
  string GetJSON();

private:
  /// Collect nodes in the transitive close of root
  void Traverse(Node* root);

  /// Creates a JSON document from all the nodes
  void DumpNodes();

  /// Creates the JSON value for a Node
  rapidjson::Value Serialize(Node* node);

  /// Node serializers
  void SerializeFloatNode(rapidjson::Value& nodeValue, FloatNode* node);
  void SerializeVec2Node(rapidjson::Value& nodeValue, Vec2Node* node);
  void SerializeVec3Node(rapidjson::Value& nodeValue, Vec3Node* node);
  void SerializeVec4Node(rapidjson::Value& nodeValue, Vec4Node* node);
  void SerializeFloatSplineNode(rapidjson::Value& nodeValue, FloatSplineNode* node);

  void SerializeTextureNode(rapidjson::Value& nodeValue, TextureNode* node);
  void SerializeStubNode(rapidjson::Value& nodeValue, StubNode* node);
  void SerializeStaticMeshNode(rapidjson::Value& nodeValue, StaticMeshNode* node);
  void SerializeGeneralNode(rapidjson::Value& nodeValue, Node* node);

  /// Helpers
  rapidjson::Value SerializeVec2(const Vec2& vec);
  rapidjson::Value SerializeVec3(const Vec3& vec);
  rapidjson::Value SerializeVec4(const Vec4& vec);

  void SerializeValueSlot(rapidjson::Value& slotsObject, Slot* slot,
                          rapidjson::Value& defaultValue);

  /// Number of nodes to save
  int mNodeCount;

  /// All nodes to save
  unordered_map<Node*, int> mNodes;
  vector<Node*> mNodesList;

  rapidjson::Document mJsonDocument;
  rapidjson::Document::AllocatorType* mAllocator;
};