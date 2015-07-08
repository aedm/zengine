#include "jsonserializer.h"
#include <include/dom/graph.h>
#include <include/base/helpers.h>
#include <include/nodes/valuenodes.h>
#include <rapidjson/include/rapidjson/writer.h>
#include <rapidjson/include/rapidjson/prettywriter.h>
#include <rapidjson/include/rapidjson/stringbuffer.h>

JSONSerializer::JSONSerializer(Node* root) {
  mJsonDocument.SetObject();
  mAllocator = &mJsonDocument.GetAllocator();

  mNodeCount = 0;
  Traverse(root);

  //DumpDocument(root);
  //DumpGraphs();
  DumpNodes();
}

string JSONSerializer::GetJSON() {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  writer.SetIndent(' ', 2);
  mJsonDocument.Accept(writer);
  return buffer.GetString();
}

rapidjson::Value JSONSerializer::Serialize(Node* node) {
  rapidjson::Value v(rapidjson::kObjectType);

  /// Save class type
  NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(node);
  v.AddMember("node", nodeClass->mClassName, *mAllocator);

  /// Save node ID
  int nodeID = mNodes.at(node);
  v.AddMember("id", nodeID, *mAllocator);

  /// Save node name
  if (!node->GetName().empty()) {
    v.AddMember("name", node->GetName(), *mAllocator);
  }

  /// Save slots
  if (node->mSlots.size() > 0) {
    rapidjson::Value slotsObject(rapidjson::kObjectType);
    for (Slot* slot : node->mSlots) {
      ASSERT(slot->GetName().get() != nullptr && !slot->GetName()->empty());
      if (slot->mIsMultiSlot) {
        rapidjson::Value connections(rapidjson::kArrayType);
        for (Node* connectedNode : slot->GetMultiNodes()) {
          int connectedID = mNodes.at(connectedNode);
          connections.PushBack(connectedID, *mAllocator);
        }
        slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
                              connections, *mAllocator);
      } else {
        Node* connectedNode = slot->GetNode();
        rapidjson::Value slotValue(rapidjson::kObjectType);
        if (IsInstanceOf<FloatSlot>(slot)) {
          float f = static_cast<FloatSlot*>(slot)->GetDefaultValue();
          slotValue.AddMember("default", f, *mAllocator);
          if (node != nullptr && !slot->IsDefaulted()) {
            int connectedID = mNodes.at(connectedNode);
            slotValue.AddMember("id", connectedID, *mAllocator);
          }
          slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
                                slotValue, *mAllocator);
        } else {
          if (connectedNode != nullptr && !slot->IsDefaulted()) {
            int connectedID = mNodes.at(connectedNode);
            slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
                                  connectedID, *mAllocator);
          }
        }
      }
    }
    v.AddMember("slots", slotsObject, *mAllocator);
  }
  return v;
}

void JSONSerializer::Traverse(Node* root) {
  mNodes[root] = ++mNodeCount;
  mNodesByClass.emplace(NodeRegistry::GetInstance()->GetNodeClass(root), root);
  mNodesList.push_back(root);

  /// Traverse slots
  for (Slot* slot : root->mSlots) {
    if (slot->mIsMultiSlot) {
      for (Node* node : slot->GetMultiNodes()) {
        auto it = mNodes.find(node);
        if (it == mNodes.end()) {
          Traverse(node);
        }
      }
    } else {
      if (slot->IsDefaulted()) continue;
      Node* node = slot->GetNode();
      auto it = mNodes.find(node);
      if (it == mNodes.end()) {
        Traverse(node);
      }
    }
  }
}

void JSONSerializer::DumpDocument(Node* root) {
  NodeClass* documentClass = NodeRegistry::GetInstance()->GetNodeClass<Document>();
  auto range = mNodesByClass.equal_range(documentClass);
  auto it = range.first;
  ASSERT(IsInstanceOf<Document>(root) == (it != mNodesByClass.end()));
  if (IsInstanceOf<Document>(root)) {
    ASSERT(it->second == root);
    ASSERT(++it == range.second);
    mJsonDocument.AddMember("document", Serialize(root), *mAllocator);
  }
}

void JSONSerializer::DumpGraphs() {
  rapidjson::Value graphArray(rapidjson::kArrayType);
  NodeClass* graphClass = NodeRegistry::GetInstance()->GetNodeClass<Graph>();
  auto range = mNodesByClass.equal_range(graphClass);
  for (auto it = range.first; it != range.second; it++) {
    Node* node = it->second;
    graphArray.PushBack(Serialize(node), *mAllocator);
  }
  mJsonDocument.AddMember("graphs", graphArray, *mAllocator);
}

void JSONSerializer::DumpNodes() {
  rapidjson::Value nodesArray(rapidjson::kArrayType);
  for (Node* node : mNodesList) {
    nodesArray.PushBack(Serialize(node), *mAllocator);
  }
  mJsonDocument.AddMember("nodes", nodesArray, *mAllocator);
}

