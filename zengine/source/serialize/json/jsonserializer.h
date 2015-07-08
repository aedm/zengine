#pragma once

#include <include/dom/document.h>
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
  rapidjson::Value Serialize(Node* node);
  
  /// Add Document node to JSON
  void DumpDocument(Node* root);

  /// Add Graph nodes to JSON
  void DumpGraphs();

  /// Adds the rest of nodes
  void DumpNodes();

  /// Collect nodes in the transitive close of root
  void Traverse(Node* root);
  
  /// Number of nodes to save
  int mNodeCount;

  /// All nodes to save
  unordered_map<Node*, int> mNodes;
  vector<Node*> mNodesList;
  unordered_multimap<NodeClass*, Node*> mNodesByClass;

  rapidjson::Document mJsonDocument;
  rapidjson::Document::AllocatorType* mAllocator;
};