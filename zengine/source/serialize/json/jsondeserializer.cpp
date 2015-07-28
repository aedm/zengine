#include "jsondeserializer.h"
#include "jsonserializer.h"
#include <include/resources/resourcemanager.h>
#include "base64/base64.h"

JSONDeserializer::JSONDeserializer(const string& json) {
  rapidjson::Document d;
  d.Parse(json.c_str());

  rapidjson::Value& jsonNodes = d["nodes"];
  ASSERT(jsonNodes.IsArray());

  INFO("Loading nodes...");
  for (UINT i = 0; i < jsonNodes.Size(); i++) {
    DeserializeNode(jsonNodes[i]);
  }

  INFO("Loading connections...");
  for (UINT i = 0; i < jsonNodes.Size(); i++) {
    ConnectSlots(jsonNodes[i]);
  }

  INFO("Loading done.");
}

Document* JSONDeserializer::GetDocument() {
  ASSERT(mDocument);
  return mDocument;
}

void JSONDeserializer::DeserializeNode(rapidjson::Value& value) {
  const char* nodeName = value["node"].GetString();
  int id = value["id"].GetInt();
  NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(string(nodeName));

  Node* node = nodeClass->Manufacture();
  ASSERT(mNodes.find(id) == mNodes.end());
  mNodes[id] = node;

  if (value.HasMember("position")) {
    Vec2 position = DeserializeVec2(value["position"]);
    node->SetPosition(position);
  }

  //rapidjson::Value& jsonPosition = value["size"];
  //if (!jsonPosition.IsNull()) {
  //  Vec2 position = DeserializeVec2(jsonPosition);
  //  node->SetPosition(position);
  //}
  if (IsInstanceOf<TextureNode>(node)) {
    DeserializeTextureNode(value, static_cast<TextureNode*>(node));
  }
  else if (IsInstanceOf<Document>(node)) {
    ASSERT(mDocument == nullptr);
    mDocument = static_cast<Document*>(node);
  }
}

Vec2 JSONDeserializer::DeserializeVec2(rapidjson::Value& value) {
  float x = value["x"].GetDouble();
  float y = value["y"].GetDouble();
  return Vec2(x, y);
}

void JSONDeserializer::ConnectSlots(rapidjson::Value& value) {
  int id = value["id"].GetInt();
  Node* node = mNodes.at(id);
  
  if (value.HasMember("slots")) {
    rapidjson::Value& jsonSlots = value["slots"];
    for (rapidjson::Value::ConstMemberIterator itr = jsonSlots.MemberBegin();
         itr != jsonSlots.MemberEnd(); ++itr) {
      string slotName(itr->name.GetString());
      Slot* slot = nullptr;
      for (Slot* s : node->GetPublicSlots()) {
        if (*s->GetName() == slotName) {
          slot = s;
          break;
        }
      }
      if (slot == nullptr) {
        ERR("No such slot: %s", slotName.c_str());
        continue;;
      }
      ASSERT(slot->mIsMultiSlot == itr->value.IsArray());
      if (IsInstanceOf<StringSlot>(slot)) {
        static_cast<StringSlot*>(slot)->
          SetDefaultValue(itr->value["default"].GetString());
      } 
      else if (dynamic_cast<FloatSlot*>(slot) != nullptr) {
        static_cast<FloatSlot*>(slot)->
          SetDefaultValue(itr->value["default"].GetDouble());
      }
      else if (itr->value.IsArray()) {
        for (UINT i = 0; i < itr->value.Size(); i++) {
          int connId = itr->value[i].GetInt();
          Node* connNode = mNodes.at(connId);
          slot->Connect(connNode);
        }
      }
      else {
        int connId = itr->value.GetInt();
        Node* connNode = mNodes.at(connId);
        slot->Connect(connNode);
      }
    }
  }
}

void JSONDeserializer::DeserializeTextureNode(const rapidjson::Value& value, 
                                              TextureNode* node) 
{
  int width = value["width"].GetInt();
  int height = value["height"].GetInt();
  const char* typeString = value["type"].GetString();
  const char* texelString = value["base64"].GetString();
  int texelTypeInt = EnumMapperA::GetEnumFromString(TexelTypeMapper, typeString);
  if (texelTypeInt < 0) {
    ERR("Unknown texture type: %s", typeString);
  }
  TexelTypeEnum texelType = (TexelTypeEnum)texelTypeInt;
  UINT byteSize = width * height * Texture::GetTexelByteCount(texelType);
  char* texels = new char[byteSize];

  string texelContent = base64_decode(texelString);
  ASSERT(byteSize == texelContent.length());
  memcpy(texels, texelContent.c_str(), byteSize);

  Texture* texture = TheResourceManager->CreateTexture(width, height, texelType, texels);
  node->Set(texture);
}

