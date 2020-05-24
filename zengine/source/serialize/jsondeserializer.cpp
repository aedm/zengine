#include "base64/base64.h"
#include <include/serialize/jsondeserializer.h>
#include <include/serialize/jsonserializer.h>
#include <include/dom/ghost.h>
#include <algorithm>
#include <memory>

JSONDeserializer::JSONDeserializer(const std::shared_ptr<Document>& document)
  : mDocument(document)
{}

void JSONDeserializer::SetDocumentJson(const std::string& documentJson) {
  ASSERT(mDocument == nullptr);

  rapidjson::Document d;
  d.Parse(documentJson.c_str());

  INFO("Loading document...");
  rapidjson::Value& documentObject = d["document"];
  DeserializeNode(documentObject);

  INFO("Loading properties...");
  rapidjson::Value& propertiesObject = d["properties"];
  DeserializeNode(propertiesObject);

  INFO("Loading movie timeline...");
  rapidjson::Value& movieObject = d["movie"];
  DeserializeNode(movieObject);

  INFO("Loading done.");
}

void JSONDeserializer::AddGraphJson(const std::string& graphJson) {
  rapidjson::Document& d = mJsonDocuments.emplace_back();
  d.Parse(graphJson.c_str());

  INFO("Loading graph...");
  rapidjson::Value& graphObject = d["graph"];
  DeserializeNode(graphObject);

  rapidjson::Value& jsonNodes = d["nodes"];
  ASSERT(jsonNodes.IsArray());

  INFO("Loading graph nodes...");
  for (UINT i = 0; i < jsonNodes.Size(); i++) {
    DeserializeNode(jsonNodes[i]);
  }

  //INFO("Loading connections...");
  //for (UINT i = 0; i < jsonNodes.Size(); i++) {
  //  ConnectSlots(jsonNodes[i]);
  //}

  INFO("Loading done.");
}

std::shared_ptr<Document> JSONDeserializer::GetDocument()
{
  ASSERT(mDocument);
  ConnectNodes();
  return mDocument;
}

void JSONDeserializer::LoadNode(rapidjson::Value& value) {
  unique_ptr<SerializedNode> serializedNode = std::make_unique<SerializedNode>();
  serializedNode->mJsonValue = &value;
  if (!value.HasMember("slots")) return;
  rapidjson::Value& jsonSlots = value["slots"];
  for (rapidjson::Value::ConstMemberIterator itr = jsonSlots.MemberBegin();
    itr != jsonSlots.MemberEnd(); ++itr) {
    const rapidjson::Value& jsonSlot = itr->value;

    /// Connect to nodes
    if (jsonSlot.HasMember("connect")) {
      const rapidjson::Value& jsonConnect = jsonSlot["connect"];
      if (jsonConnect.IsArray()) {
        for (UINT i = 0; i < jsonConnect.Size(); i++) {
          serializedNode->mRequiredIds.insert(jsonConnect[i].GetString());
        }
      }
      else {
        serializedNode->mRequiredIds.insert(jsonConnect.GetString());
      }
    }
  }
}

void JSONDeserializer::ConnectNodes() {
  while (!mUnconnectedNodes.empty()) {
    for (const auto& it : mUnconnectedNodes) {
      /// Check whether node can be connected
      bool connectable = true;
      for (const auto& connection : it->mConnections) {
        
      }
    }
  }
}


void JSONDeserializer::DeserializeNode(rapidjson::Value& value) {
  ASSERT(value.IsObject());
  const std::string nodeClassName = value["node"].GetString();
  const std::string id = value["id"].GetString();
  ASSERT(mNodesById.find(id) == mNodesById.end());

  std::shared_ptr<Node> node;
  if (nodeClassName == "ghost") {
    node = std::make_shared<Ghost>();
  }
  else {
    NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(nodeClassName);
    node = nodeClass->Manufacture();
  }
  mNodesById[id] = node;

  if (value.HasMember("name")) {
    node->SetName(value["name"].GetString());
  }

  if (value.HasMember("position")) {
    const vec2 position = DeserializeVec2(value["position"]);
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

  AnalyzeSlots(node, value);
}

void JSONDeserializer::AnalyzeSlots(const shared_ptr<Node>& node,
  rapidjson::Value& value)
{
  if (!value.HasMember("slots")) return;

  const auto nodeConnections = std::make_unique<NodeConnections>();
  mUnconnectedNodes.insert(nodeConnections);
  nodeConnections->mNode = node;

  rapidjson::Value& jsonSlots = value["slots"];

  for (rapidjson::Value::ConstMemberIterator itr = jsonSlots.MemberBegin();
    itr != jsonSlots.MemberEnd(); ++itr) {
    const rapidjson::Value& jsonSlot = itr->value;

    auto& nodeConnection = nodeConnections->mConnections.emplace_back();
    nodeConnection.mSlotName = itr->name.GetString();

    /// Set ghost flag
    if (jsonSlot.HasMember("ghost")) {
      nodeConnection.mIsGhostSlot = jsonSlot["ghost"].GetBool();
    }

    /// Connect to nodes
    if (jsonSlot.HasMember("connect")) {
      const rapidjson::Value& jsonConnect = jsonSlot["connect"];
      if (jsonConnect.IsArray()) {
        for (UINT i = 0; i < jsonConnect.Size(); i++) {
          nodeConnection.mNodeIds.emplace_back(jsonConnect[i].GetString());
        }
      }
      else {
        nodeConnection.mNodeIds.emplace_back(jsonConnect.GetString());
      }
    }

    if (jsonSlot.HasMember("default")) {
      nodeConnection.mDefault = &jsonSlot["default"];
    }
  }
}

vec2 JSONDeserializer::DeserializeVec2(const rapidjson::Value& value) {
  const float x = float(value["x"].GetDouble());
  const float y = float(value["y"].GetDouble());
  return vec2(x, y);
}


vec3 JSONDeserializer::DeserializeVec3(const rapidjson::Value& value) {
  const float x = float(value["x"].GetDouble());
  const float y = float(value["y"].GetDouble());
  const float z = float(value["z"].GetDouble());
  return vec3(x, y, z);
}


vec4 JSONDeserializer::DeserializeVec4(const rapidjson::Value& value) {
  const float x = float(value["x"].GetDouble());
  const float y = float(value["y"].GetDouble());
  const float z = float(value["z"].GetDouble());
  const float w = float(value["w"].GetDouble());
  return vec4(x, y, z, w);
}

void JSONDeserializer::ConnectSlots2(rapidjson::Value& value) {
  const std::string id = value["id"].GetString();
  const std::shared_ptr<Node> node = mNodesById.at(id);
  const auto& slots = node->GetSerializableSlots();

  if (value.HasMember("slots")) {
    rapidjson::Value& jsonSlots = value["slots"];

    if (IsPointerOf<Ghost>(node)) {
      /// Connect original node first
      if (jsonSlots.HasMember("Original")) {
        const rapidjson::Value& jsonSlot = jsonSlots["Original"];
        if (jsonSlot.HasMember("connect")) {
          const rapidjson::Value& jsonConnect = jsonSlot["connect"];
          const std::string connId = jsonConnect.GetString();
          const std::shared_ptr<Node> connNode = mNodesById.at(connId);
          PointerCast<Ghost>(node)->mOriginalNode.Connect(connNode);
        }
      }
    }

    for (rapidjson::Value::ConstMemberIterator itr = jsonSlots.MemberBegin();
      itr != jsonSlots.MemberEnd(); ++itr) {
      const rapidjson::Value& jsonSlot = itr->value;

      /// Find slot
      std::string slotName(itr->name.GetString());
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
            std::string connId = jsonConnect[i].GetString();
            std::shared_ptr<Node> connNode = mNodesById.at(connId);
            slot->Connect(connNode);
          }
        }
        else {
          std::string connId = jsonConnect.GetString();
          std::shared_ptr<Node> connNode = mNodesById.at(connId);
          slot->Connect(connNode);
        }
      }

      /// Set default values
      if (dynamic_cast<StringSlot*>(slot)) {
        SafeCast<StringSlot*>(slot)->SetDefaultValue(jsonSlot["default"].GetString());
      }
      else if (dynamic_cast<FloatSlot*>(slot)) {
        SafeCast<FloatSlot*>(slot)->SetDefaultValue(
          float(jsonSlot["default"].GetDouble()));
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
                                                    const std::shared_ptr<StaticTextureNode>& node) const
{
  const int width = value["width"].GetInt();
  const int height = value["height"].GetInt();
  const char* typeString = value["type"].GetString();
  const char* texelString = value["base64"].GetString();
  const TexelType texelType = TexelTypeMapper.GetEnum(typeString);
  if (signed(texelType) < 0) {
    ERR("Unknown texture type: %s", typeString);
  }
  const std::string texelContent = base64_decode(texelString);
  const std::shared_ptr<Texture> texture = OpenGL->MakeTexture(width, height, texelType, 
    texelContent.c_str(), false, false, true, true);
  node->Set(texture);
}


void JSONDeserializer::DeserializeStaticMeshNode(const rapidjson::Value& value,
  const std::shared_ptr<StaticMeshNode>& node) const
{
  int binaryFormat = value["format"].GetInt();
  const UINT vertexCount = value["vertexcount"].GetInt();
  const std::shared_ptr<VertexFormat> format = std::make_shared<VertexFormat>(binaryFormat);
  std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();

  mesh->AllocateVertices(format, vertexCount);
  std::vector<float> rawVertices(vertexCount * format->mStride / sizeof(float));
  const rapidjson::Value& jsonVertices = value["vertices"];
  for (UINT i = 0; i < jsonVertices.Size(); i++) {
    rawVertices[i] = float(jsonVertices[i].GetDouble());
  }
  mesh->UploadVertices(&rawVertices[0]);

  if (value.HasMember("indices")) {
    const UINT indexCount = value["indexcount"].GetInt();
    mesh->AllocateIndices(indexCount);
    std::vector<IndexEntry> indices(indexCount);
    const rapidjson::Value& jsonIndices = value["indices"];
    for (UINT i = 0; i < jsonIndices.Size(); i++) {
      indices[i] = IndexEntry(jsonIndices[i].GetUint());
    }
    mesh->UploadIndices(&indices[0]);
  }

  node->Set(mesh);
}


void JSONDeserializer::DeserializeFloatSplineNode(const rapidjson::Value& value,
  const std::shared_ptr<FloatSplineNode>& node)
{
  for (UINT l = UINT(SplineLayer::BASE); l < UINT(SplineLayer::COUNT); l++) {
    const char* fieldName = SplineLayerMapper.GetName(SplineLayer(l));
    if (!value.HasMember(fieldName)) continue;

    const SplineLayer layer = SplineLayer(l);
    const rapidjson::Value& jsonPoints = value[fieldName];
    for (UINT i = 0; i < jsonPoints.Size(); i++) {
      auto& jsonPoint = jsonPoints[i];
      const float time = float(jsonPoint["time"].GetDouble());
      const float floatValue = float(jsonPoint["value"].GetDouble());
      const int pIndex = node->AddPoint(layer, time, floatValue);
      node->SetAutoTangent(layer, pIndex, jsonPoint["autotangent"].GetBool());
      node->SetBreakpoint(layer, pIndex, jsonPoint["breakpoint"].GetBool());
      node->SetLinear(layer, pIndex, jsonPoint["linear"].GetBool());
    }
  }
}


void JSONDeserializer::DeserializeStubNode(const rapidjson::Value& value,
  const std::shared_ptr<StubNode>& node)
{
  const std::string source = value["source"].GetString();
  node->mSource.SetDefaultValue(source);
  node->Update();
}


//void JSONDeserializer::ConnectValueSlotById(const rapidjson::Value& value, Slot* slot) {
//  if (value.HasMember("id")) {
//    const std::string connId = value["id"].GetString();
//    auto& connNode = mNodesById.at(connId);
//    slot->Connect(connNode);
//  }
//}

void JSONDeserializer::DeserializeFloatNode(const rapidjson::Value& value,
  const std::shared_ptr<FloatNode>& node) {
  node->Set(float(value["value"].GetDouble()));
}

void JSONDeserializer::DeserializeVec2Node(const rapidjson::Value& value,
  const std::shared_ptr<Vec2Node>& node) {
  node->Set(DeserializeVec2(value["value"]));
}

void JSONDeserializer::DeserializeVec3Node(const rapidjson::Value& value,
  const std::shared_ptr<Vec3Node>& node) {
  node->Set(DeserializeVec3(value["value"]));
}

void JSONDeserializer::DeserializeVec4Node(const rapidjson::Value& value,
  const std::shared_ptr<Vec4Node>& node) {
  node->Set(DeserializeVec4(value["value"]));
}


