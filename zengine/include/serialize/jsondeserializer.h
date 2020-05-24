#pragma once

#include "../dom/document.h"
#include "../nodes/valuenodes.h"
#include "../nodes/meshnode.h"
#include "../nodes/texturenode.h"
#include "../nodes/splinenode.h"
#include "../shaders/stubnode.h"
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>

using std::vector;
using std::shared_ptr;
using std::pair;
using std::string;
using std::set;
using std::unique_ptr;

class JSONDeserializer {
public:
  JSONDeserializer(const std::shared_ptr<Document>& document);
  void SetDocumentJson(const std::string& documentJson);
  void AddGraphJson(const std::string& graphJson);
  std::shared_ptr<Document> GetDocument();

private:
  void LoadNode(rapidjson::Value& value);
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

  void ConnectSlots2(rapidjson::Value& value);
  void ConnectNodes();

  /// Extract connections and put the node into the unconnected pool
  void AnalyzeSlots(const shared_ptr<Node>& node, rapidjson::Value& value);

  ///// Connects a slot by "id" tag.
  //void ConnectValueSlotById(const rapidjson::Value& value, Slot* slot);

  static vec2 DeserializeVec2(const rapidjson::Value& value);
  static vec3 DeserializeVec3(const rapidjson::Value& value);
  static vec4 DeserializeVec4(const rapidjson::Value& value);

  std::unordered_map<string, shared_ptr<Node>> mNodesById;

  //struct NodeConnection {
  //  string mSlotName;
  //  vector<string> mNodeIds;
  //  bool mIsGhostSlot = false;
  //  const rapidjson::Value* mDefault = nullptr;
  //};

  //struct NodeConnections {
  //  std::shared_ptr<Node> mNode;
  //  vector<NodeConnection> mConnections;
  //};

  //set<unique_ptr<NodeConnections>> mUnconnectedNodes;

  struct SerializedNode {
    set<string> mRequiredIds;
    const rapidjson::Value* mJsonValue = nullptr;
  };

  set<unique_ptr<SerializedNode>> mSerializedNodes;

  std::shared_ptr<Document> mDocument;
  std::vector<rapidjson::Document> mJsonDocuments;
};