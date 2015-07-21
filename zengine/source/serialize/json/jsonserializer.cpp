#include "jsonserializer.h"
#include <include/dom/graph.h>
#include <include/base/helpers.h>
#include <include/nodes/valuenodes.h>
#include <include/shaders/valuestubslot.h>
#include "base64/base64.h"
#include <rapidjson/include/rapidjson/writer.h>
#include <rapidjson/include/rapidjson/prettywriter.h>
#include <rapidjson/include/rapidjson/stringbuffer.h>

const EnumMapperA TexelTypeMapper[] = {
  {"RGBA8", UINT(TEXELTYPE_RGBA_UINT8)},
  {"RGBA16", UINT(TEXELTYPE_RGBA_UINT16)},
  {"RGBA16F", UINT(TEXELTYPE_RGBA_FLOAT16)},
  {"RGBA32F", UINT(TEXELTYPE_RGBA_FLOAT32)},
  {"", -1}
};

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
  //for (Slot* slot : root->mSlots) {
  for (auto slotPair : root->GetSerializableSlots()) {
    Slot* slot = slotPair.second;
    if (slot->mIsMultiSlot) {
      for (Node* node : slot->GetMultiNodes()) {
        auto it = mNodes.find(node);
        if (it == mNodes.end()) {
          Traverse(node);
        }
      }
    } else {
      Node* node = slot->GetAbstractNode();
      if (node == nullptr || slot->IsDefaulted()) continue;
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
  else if (IsInstanceOf<TextureNode>(node)) {
    SerializeTextureNode(v, static_cast<TextureNode*>(node));
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
  if (node->GetSerializableSlots().size() > 0) {
    rapidjson::Value slotsObject(rapidjson::kObjectType);
    for (auto slotPair : node->GetSerializableSlots()) {
      Slot* slot = slotPair.second;
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
        Node* connectedNode = slot->GetAbstractNode();

        /// Save FloatSlot
        FloatSlot* floatSlot;
        StringSlot* stringSlot;
        if ((floatSlot = dynamic_cast<FloatSlot*>(slot)) != nullptr) {
          rapidjson::Value slotValue(rapidjson::kObjectType);
          float f = floatSlot->GetDefaultValue();
          slotValue.AddMember("default", f, *mAllocator);
          if (node != nullptr && !slot->IsDefaulted()) {
            int connectedID = mNodes.at(connectedNode);
            slotValue.AddMember("id", connectedID, *mAllocator);
          }
          slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
            slotValue, *mAllocator);
        } 
        
        /// Save StringSlot
        else if ((stringSlot = dynamic_cast<StringSlot*>(slot)) != nullptr) {
          const string& f = stringSlot->GetDefaultValue();
          rapidjson::Value slotValue(rapidjson::kObjectType);
          slotValue.AddMember("default", f, *mAllocator);
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

void JSONSerializer::SerializeTextureNode(rapidjson::Value& nodeValue, TextureNode* node) {
  Texture* texture = node->Get();
  ASSERT(texture->mTexelData);
  string b64 = base64_encode((UCHAR*)texture->mTexelData, texture->mTexelDataByteCount);
  nodeValue.AddMember("width", texture->mWidth, *mAllocator);
  nodeValue.AddMember("height", texture->mHeight, *mAllocator);
  nodeValue.AddMember("type", rapidjson::Value(
    EnumMapperA::GetStringFromEnum(TexelTypeMapper, texture->mType), *mAllocator), 
    *mAllocator);
  nodeValue.AddMember("base64", b64, *mAllocator);
}

