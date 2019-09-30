#include <algorithm>
#include "jsondeserializer.h"
#include "jsonserializer.h"
#include "base64/base64.h"
#include <include/dom/ghost.h>

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

shared_ptr<Document> JSONDeserializer::GetDocument() const
{
  ASSERT(mDocument);
  return mDocument;
}

void JSONDeserializer::DeserializeNode(rapidjson::Value& value) {
  const string nodeClassName = value["node"].GetString();
  const int id = value["id"].GetInt();
  ASSERT(mNodes.find(id) == mNodes.end());

  shared_ptr<Node> node;
  if (nodeClassName == "ghost") {
    node = make_shared<Ghost>();
  }
  else {
    NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(nodeClassName);
    node = nodeClass->Manufacture();
  }
  mNodes[id] = node;

  if (value.HasMember("name")) {
    node->SetName(value["name"].GetString());
  }

  if (value.HasMember("position")) {
    const Vec2 position = DeserializeVec2(value["position"]);
    node->SetPosition(position);
  }

  if (IsExactType<FloatNode>(node)) {
    DeserializeFloatNode(value, PointerCast<FloatNode>(node));
  }
  else if (IsExactType<Vec2Node>(node)) {
    DeserializeVec2Node(value, PointerCast<Vec2Node>(node));
  }
  else if (IsExactType<Vec3Node>(node)) {
    DeserializeVec3Node(value, PointerCast<Vec3Node>(node));
  }
  else if (IsExactType<Vec4Node>(node)) {
    DeserializeVec4Node(value, PointerCast<Vec4Node>(node));
  }
  else if (IsExactType<FloatSplineNode>(node)) {
    DeserializeFloatSplineNode(value, PointerCast<FloatSplineNode>(node));
  }
  else if (IsExactType<StaticTextureNode>(node)) {
    DeserializeStaticTextureNode(value, PointerCast<StaticTextureNode>(node));
  }
  else if (IsExactType<StaticMeshNode>(node)) {
    DeserializeStaticMeshNode(value, PointerCast<StaticMeshNode>(node));
  }
  else if (IsExactType<StubNode>(node)) {
    DeserializeStubNode(value, PointerCast<StubNode>(node));
  }
  else if (IsExactType<Document>(node)) {
    ASSERT(mDocument == nullptr);
    mDocument = PointerCast<Document>(node);
  }
}

Vec2 JSONDeserializer::DeserializeVec2(const rapidjson::Value& value) {
  const float x = value["x"].GetDouble();
  const float y = value["y"].GetDouble();
  return Vec2(x, y);
}


Vec3 JSONDeserializer::DeserializeVec3(const rapidjson::Value& value) {
  const float x = value["x"].GetDouble();
  const float y = value["y"].GetDouble();
  const float z = value["z"].GetDouble();
  return Vec3(x, y, z);
}


Vec4 JSONDeserializer::DeserializeVec4(const rapidjson::Value& value) {
  const float x = value["x"].GetDouble();
  const float y = value["y"].GetDouble();
  const float z = value["z"].GetDouble();
  const float w = value["w"].GetDouble();
  return Vec4(x, y, z, w);
}

void JSONDeserializer::ConnectSlots(rapidjson::Value& value) {
  const int id = value["id"].GetInt();
  const shared_ptr<Node> node = mNodes.at(id);
  const auto& slots = node->GetSerializableSlots();

  if (value.HasMember("slots")) {
    rapidjson::Value& jsonSlots = value["slots"];

    if (IsPointerOf<Ghost>(node)) {
      /// Connect original node first
      if (jsonSlots.HasMember("Original")) {
        const rapidjson::Value& jsonSlot = jsonSlots["Original"];
        if (jsonSlot.HasMember("connect")) {
          const rapidjson::Value& jsonConnect = jsonSlot["connect"];
          const int connId = jsonConnect.GetInt();
          const shared_ptr<Node> connNode = mNodes.at(connId);
          PointerCast<Ghost>(node)->mOriginalNode.Connect(connNode);
        }
      }
    }

    for (rapidjson::Value::ConstMemberIterator itr = jsonSlots.MemberBegin();
      itr != jsonSlots.MemberEnd(); ++itr) {
      const rapidjson::Value& jsonSlot = itr->value;

      /// Find slot
      string slotName(itr->name.GetString());
      const auto& it = std::find_if(slots.begin(), slots.end(),
        [&](const auto& m) -> bool { return slotName == m.first; });
      if (it == slots.end()) {
        ERR("No such slot: %s", slotName.c_str());
        continue;;
      }
      Slot* slot = it->second;

      /// Set ghost flag
      if (jsonSlot.HasMember("ghost")) {
        slot->SetGhost(jsonSlot["ghost"].GetBool());
      }

      /// Connect to nodes
      if (jsonSlot.HasMember("connect")) {
        const rapidjson::Value& jsonConnect = jsonSlot["connect"];
        if (slot->mIsMultiSlot != jsonConnect.IsArray()) {
          ERR("Multi/single slot mismatch: %s", slotName.c_str());
          continue;
        }
        if (jsonConnect.IsArray()) {
          for (UINT i = 0; i < jsonConnect.Size(); i++) {
            int connId = jsonConnect[i].GetInt();
            shared_ptr<Node> connNode = mNodes.at(connId);
            slot->Connect(connNode);
          }
        }
        else {
          int connId = jsonConnect.GetInt();
          shared_ptr<Node> connNode = mNodes.at(connId);
          slot->Connect(connNode);
        }
      }

      /// Set default values
      if (dynamic_cast<StringSlot*>(slot)) {
        SafeCast<StringSlot*>(slot)->SetDefaultValue(jsonSlot["default"].GetString());
      }
      else if (dynamic_cast<FloatSlot*>(slot)) {
        SafeCast<FloatSlot*>(slot)->SetDefaultValue(jsonSlot["default"].GetDouble());
      }
      else if (dynamic_cast<Vec2Slot*>(slot)) {
        SafeCast<Vec2Slot*>(slot)->
          SetDefaultValue(DeserializeVec2(itr->value["default"]));
      }
      else if (dynamic_cast<Vec3Slot*>(slot)) {
        SafeCast<Vec3Slot*>(slot)->
          SetDefaultValue(DeserializeVec3(itr->value["default"]));
      }
      else if (dynamic_cast<Vec4Slot*>(slot)) {
        SafeCast<Vec4Slot*>(slot)->
          SetDefaultValue(DeserializeVec4(itr->value["default"]));
      }
    }
  }
}

void JSONDeserializer::DeserializeStaticTextureNode(const rapidjson::Value& value,
  const shared_ptr<StaticTextureNode>& node) const
{
  const int width = value["width"].GetInt();
  const int height = value["height"].GetInt();
  const char* typeString = value["type"].GetString();
  const char* texelString = value["base64"].GetString();
  int texelTypeInt = EnumMapperA::GetEnumFromString(TexelTypeMapper, typeString);
  if (texelTypeInt < 0) {
    ERR("Unknown texture type: %s", typeString);
  }
  const TexelType texelType = TexelType(texelTypeInt);
  const string texelContent = base64_decode(texelString);
  const shared_ptr<Texture> texture = OpenGL->MakeTexture(width, height, texelType, 
    texelContent.c_str(), false, false, true, true);
  node->Set(texture);
}


void JSONDeserializer::DeserializeStaticMeshNode(const rapidjson::Value& value,
  const shared_ptr<StaticMeshNode>& node) const
{
  int binaryFormat = value["format"].GetInt();
  const UINT vertexCount = value["vertexcount"].GetInt();
  const shared_ptr<VertexFormat> format = make_shared<VertexFormat>(binaryFormat);
  shared_ptr<Mesh> mesh = make_shared<Mesh>();

  mesh->AllocateVertices(format, vertexCount);
  vector<float> rawVertices(vertexCount * format->mStride / sizeof(float));
  const rapidjson::Value& jsonVertices = value["vertices"];
  for (UINT i = 0; i < jsonVertices.Size(); i++) {
    rawVertices[i] = float(jsonVertices[i].GetDouble());
  }
  mesh->UploadVertices(&rawVertices[0]);

  if (value.HasMember("indices")) {
    const UINT indexCount = value["indexcount"].GetInt();
    mesh->AllocateIndices(indexCount);
    vector<IndexEntry> indices(indexCount);
    const rapidjson::Value& jsonIndices = value["indices"];
    for (UINT i = 0; i < jsonIndices.Size(); i++) {
      indices[i] = IndexEntry(jsonIndices[i].GetUint());
    }
    mesh->UploadIndices(&indices[0]);
  }

  node->Set(mesh);
}


void JSONDeserializer::DeserializeFloatSplineNode(const rapidjson::Value& value,
  const shared_ptr<FloatSplineNode>& node)
{
  for (UINT l = UINT(SplineLayer::BASE); l < UINT(SplineLayer::COUNT); l++) {
    const char* fieldName = EnumMapperA::GetStringFromEnum(SplineLayerMapper, l);
    if (!value.HasMember(fieldName)) continue;

    const SplineLayer layer = SplineLayer(l);
    const rapidjson::Value& jsonPoints = value[fieldName];
    for (UINT i = 0; i < jsonPoints.Size(); i++) {
      auto& jsonPoint = jsonPoints[i];
      const float time(jsonPoint["time"].GetDouble());
      const float floatValue(jsonPoint["value"].GetDouble());
      const int pIndex = node->AddPoint(layer, time, floatValue);
      node->SetAutoTangent(layer, pIndex, jsonPoint["autotangent"].GetBool());
      node->SetBreakpoint(layer, pIndex, jsonPoint["breakpoint"].GetBool());
      node->SetLinear(layer, pIndex, jsonPoint["linear"].GetBool());
    }
  }
}


void JSONDeserializer::DeserializeStubNode(const rapidjson::Value& value,
  const shared_ptr<StubNode>& node)
{
  const string source = value["source"].GetString();
  node->mSource.SetDefaultValue(source);
  node->Update();
}


void JSONDeserializer::ConnectValueSlotById(const rapidjson::Value& value, Slot* slot) {
  if (value.HasMember("id")) {
    const int connId = value["id"].GetDouble();
    auto& connNode = mNodes.at(connId);
    slot->Connect(connNode);
  }
}

void JSONDeserializer::DeserializeFloatNode(const rapidjson::Value& value,
  const shared_ptr<FloatNode>& node) {
  node->Set(value["value"].GetDouble());
}

void JSONDeserializer::DeserializeVec2Node(const rapidjson::Value& value,
  const shared_ptr<Vec2Node>& node) {
  node->Set(DeserializeVec2(value["value"]));
}

void JSONDeserializer::DeserializeVec3Node(const rapidjson::Value& value,
  const shared_ptr<Vec3Node>& node) {
  node->Set(DeserializeVec3(value["value"]));
}

void JSONDeserializer::DeserializeVec4Node(const rapidjson::Value& value,
  const shared_ptr<Vec4Node>& node) {
  node->Set(DeserializeVec4(value["value"]));
}


