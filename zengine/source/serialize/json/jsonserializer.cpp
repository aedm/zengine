#include "jsonserializer.h"
#include <include/dom/graph.h>
#include <include/base/helpers.h>
#include <include/nodes/valuenodes.h>
#include <include/shaders/valuestubslot.h>
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
  //rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  mJsonDocument.Accept(writer);
  return buffer.GetString();
}

void JSONSerializer::Traverse(Node* root) {
  mNodes[root] = ++mNodeCount;
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

void JSONSerializer::DumpNodes() {
  rapidjson::Value nodesArray(rapidjson::kArrayType);
  for (Node* node : mNodesList) {
    nodesArray.PushBack(Serialize(node), *mAllocator);
  }
  mJsonDocument.AddMember("nodes", nodesArray, *mAllocator);
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

  /// Save graph position
  if (!IsInstanceOf<Graph>(node) && !IsInstanceOf<Document>(node)) {
    v.AddMember("position", SerializeVec2(node->GetPosition()), *mAllocator);
  }

  /// Save node content
  if (IsInstanceOf<FloatNode>(node)) {
    SerializeFloatNode(v, static_cast<FloatNode*>(node));
  }
  else {
    SerializeGeneralNode(v, node);
  }

  return v;
}

rapidjson::Value JSONSerializer::SerializeVec2(const Vec2& vec)
{
  rapidjson::Value jsonObject(rapidjson::kObjectType);
  jsonObject.AddMember("x", vec.x, *mAllocator);
  jsonObject.AddMember("y", vec.y, *mAllocator);
  return jsonObject;
}


void JSONSerializer::SerializeGeneralNode(rapidjson::Value& nodeValue, Node* node)
{
  /// Save slots
  if (node->mSlots.size() > 0) {
    rapidjson::Value slotsObject(rapidjson::kObjectType);
    for (Slot* slot : node->mSlots) {
      ASSERT(slot->GetName().get() != nullptr && !slot->GetName()->empty());

      /// Save multislot
      if (slot->mIsMultiSlot) {
        rapidjson::Value connections(rapidjson::kArrayType);
        for (Node* connectedNode : slot->GetMultiNodes()) {
          int connectedID = mNodes.at(connectedNode);
          connections.PushBack(connectedID, *mAllocator);
        }
        slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
          connections, *mAllocator);
      }
      else {
        Node* connectedNode = slot->GetNode();
        rapidjson::Value slotValue(rapidjson::kObjectType);

        /// Save FloatSlot
        FloatSlot* floatSlot = dynamic_cast<FloatSlot*>(slot);
        if (floatSlot != nullptr) {
          float f = floatSlot->GetDefaultValue();
          slotValue.AddMember("default", f, *mAllocator);
          if (node != nullptr && !slot->IsDefaulted()) {
            int connectedID = mNodes.at(connectedNode);
            slotValue.AddMember("id", connectedID, *mAllocator);
          }
          slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
            slotValue, *mAllocator);
        }

        /// Save all other kind of slots
        else {
          if (connectedNode != nullptr && !slot->IsDefaulted()) {
            int connectedID = mNodes.at(connectedNode);
            slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
              connectedID, *mAllocator);
          }
        }
      }
    }
    nodeValue.AddMember("slots", slotsObject, *mAllocator);
  }
}

void JSONSerializer::SerializeFloatNode(rapidjson::Value& nodeValue, FloatNode* node)
{
  nodeValue.AddMember("value", node->Get(), *mAllocator);
}

