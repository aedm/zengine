#pragma once

#include <include/dom/document.h>
#include <include/nodes/valuenodes.h>
#include <string>
#include <rapidjson/include/rapidjson/document.h>
#include <unordered_map>
#include <set>

using namespace std;

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
  void SerializeTextureNode(rapidjson::Value& nodeValue, TextureNode* node);
  void SerializeGeneralNode(rapidjson::Value& nodeValue, Node* node);

  /// Helpers
  rapidjson::Value SerializeVec2(const Vec2& vec);

  /// Number of nodes to save
  int mNodeCount;

  /// All nodes to save
  unordered_map<Node*, int> mNodes;
  vector<Node*> mNodesList;

  rapidjson::Document mJsonDocument;
  rapidjson::Document::AllocatorType* mAllocator;
};