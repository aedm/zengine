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
  {"RGBA8", UINT(TexelType::ARGB8)},
  {"RGBA16", UINT(TexelType::ARGB16)},
  {"RGBA16F", UINT(TexelType::ARGB16F)},
  {"RGBA32F", UINT(TexelType::ARGB32F)},
  {"", -1}
};

const EnumMapperA SplineLayerMapper[] = {
  {"base", UINT(SplineLayer::BASE)},
  {"noise", UINT(SplineLayer::NOISE)},
  {"beat_spike_intensity", UINT(SplineLayer::BEAT_SPIKE_INTENSITY)},
  {"beat_spike_frequency", UINT(SplineLayer::BEAT_SPIKE_FREQUENCY)},
  {"beat_quantizer", UINT(SplineLayer::BEAT_QUANTIZER)},
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
  writer.SetIndent(' ', 1);
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
      for (Node* node : slot->GetDirectMultiNodes()) {
        auto it = mNodes.find(node);
        if (it == mNodes.end()) {
          Traverse(node);
        }
      }
    } else {
      Node* node = slot->GetDirectNode();
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

  /// TODO: save size

  /// Save node content
  if (IsInstanceOf<FloatNode>(node)) {
    SerializeFloatNode(v, static_cast<FloatNode*>(node));
  } else if (IsInstanceOf<Vec2Node>(node)) {
    SerializeVec2Node(v, static_cast<Vec2Node*>(node));
  } else if (IsInstanceOf<Vec3Node>(node)) {
    SerializeVec3Node(v, static_cast<Vec3Node*>(node));
  } else if (IsInstanceOf<Vec4Node>(node)) {
    SerializeVec4Node(v, static_cast<Vec4Node*>(node));
  } else if (IsInstanceOf<FloatSplineNode>(node)) {
    SerializeFloatSplineNode(v, static_cast<FloatSplineNode*>(node));
  } else if (IsInstanceOf<TextureNode>(node)) {
    SerializeTextureNode(v, static_cast<TextureNode*>(node));
  } else if (IsInstanceOf<StaticMeshNode>(node)) {
    SerializeStaticMeshNode(v, static_cast<StaticMeshNode*>(node));
  } else if (IsInstanceOf<StubNode>(node)) {
    SerializeStubNode(v, static_cast<StubNode*>(node));
  }

  SerializeGeneralNode(v, node);
  return v;
}

rapidjson::Value JSONSerializer::SerializeVec2(const Vec2& vec) {
  rapidjson::Value jsonObject(rapidjson::kObjectType);
  jsonObject.AddMember("x", vec.x, *mAllocator);
  jsonObject.AddMember("y", vec.y, *mAllocator);
  return jsonObject;
}

rapidjson::Value JSONSerializer::SerializeVec3(const Vec3& vec) {
  rapidjson::Value jsonObject(rapidjson::kObjectType);
  jsonObject.AddMember("x", vec.x, *mAllocator);
  jsonObject.AddMember("y", vec.y, *mAllocator);
  jsonObject.AddMember("z", vec.z, *mAllocator);
  return jsonObject;
}

rapidjson::Value JSONSerializer::SerializeVec4(const Vec4& vec) {
  rapidjson::Value jsonObject(rapidjson::kObjectType);
  jsonObject.AddMember("x", vec.x, *mAllocator);
  jsonObject.AddMember("y", vec.y, *mAllocator);
  jsonObject.AddMember("z", vec.z, *mAllocator);
  jsonObject.AddMember("w", vec.w, *mAllocator);
  return jsonObject;
}


void JSONSerializer::SerializeGeneralNode(rapidjson::Value& nodeValue, Node* node) {
  /// Save slots
  if (node->GetSerializableSlots().size() > 0) {
    rapidjson::Value slotsObject(rapidjson::kObjectType);
    for (auto slotPair : node->GetSerializableSlots()) {
      Slot* slot = slotPair.second;
      ASSERT(slot->GetName().get() != nullptr && !slot->GetName()->empty());

      /// Save multislot
      if (slot->mIsMultiSlot) {
        rapidjson::Value connections(rapidjson::kArrayType);
        for (Node* connectedNode : slot->GetDirectMultiNodes()) {
          int connectedID = mNodes.at(connectedNode);
          connections.PushBack(connectedID, *mAllocator);
        }
        slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
                              connections, *mAllocator);
      } else {
        Node* connectedNode = slot->GetDirectNode();

        FloatSlot* floatSlot;
        Vec2Slot* vec2Slot;
        Vec3Slot* vec3Slot;
        Vec4Slot* vec4Slot;
        StringSlot* stringSlot;

        /// Save FloatSlot
        if ((floatSlot = dynamic_cast<FloatSlot*>(slot)) != nullptr) {
          rapidjson::Value defaultValue(floatSlot->GetDefaultValue());
          SerializeValueSlot(slotsObject, slot, defaultValue);
        }

        /// Save Vec2Slot
        else if ((vec2Slot = dynamic_cast<Vec2Slot*>(slot)) != nullptr) {
          rapidjson::Value defaultValue = SerializeVec2(vec2Slot->GetDefaultValue());
          SerializeValueSlot(slotsObject, slot, defaultValue);
        }

        /// Save Vec3Slot
        else if ((vec3Slot = dynamic_cast<Vec3Slot*>(slot)) != nullptr) {
          rapidjson::Value defaultValue = SerializeVec3(vec3Slot->GetDefaultValue());
          SerializeValueSlot(slotsObject, slot, defaultValue);
        }

        /// Save Vec4Slot
        else if ((vec4Slot = dynamic_cast<Vec4Slot*>(slot)) != nullptr) {
          rapidjson::Value defaultValue = SerializeVec4(vec4Slot->GetDefaultValue());
          SerializeValueSlot(slotsObject, slot, defaultValue);
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


void JSONSerializer::SerializeFloatNode(rapidjson::Value& nodeValue, FloatNode* node) {
  nodeValue.AddMember("value", node->Get(), *mAllocator);
}

void JSONSerializer::SerializeVec2Node(rapidjson::Value& nodeValue, Vec2Node* node) {
  nodeValue.AddMember("value", SerializeVec2(node->Get()), *mAllocator);
}

void JSONSerializer::SerializeVec3Node(rapidjson::Value& nodeValue, Vec3Node* node) {
  nodeValue.AddMember("value", SerializeVec3(node->Get()), *mAllocator);
}

void JSONSerializer::SerializeVec4Node(rapidjson::Value& nodeValue, Vec4Node* node) {
  nodeValue.AddMember("value", SerializeVec4(node->Get()), *mAllocator);
}

void JSONSerializer::SerializeFloatSplineNode(rapidjson::Value& nodeValue, FloatSplineNode* node) {
  for (UINT layer = UINT(SplineLayer::BASE); layer < UINT(SplineLayer::COUNT); layer++) {
    SplineFloatComponent* component = node->GetComponent(SplineLayer(layer));
    auto& points = component->GetPoints();
    rapidjson::Value pointArray(rapidjson::kArrayType);
    for (UINT i = 0; i < points.size(); i++) {
      const SplinePoint& point = points[i];
      rapidjson::Value p(rapidjson::kObjectType);
      p.AddMember("time", point.mTime, *mAllocator);
      p.AddMember("value", point.mValue, *mAllocator);
      p.AddMember("autotangent", point.mIsAutoangent, *mAllocator);
      p.AddMember("breakpoint", point.mIsBreakpoint, *mAllocator);
      p.AddMember("linear", point.mIsLinear, *mAllocator);
      pointArray.PushBack(p, *mAllocator);
    }
    const char* fieldName = EnumMapperA::GetStringFromEnum(SplineLayerMapper, layer);
    nodeValue.AddMember(rapidjson::GenericStringRef<char>(fieldName), pointArray,
                        *mAllocator);
  }
}

void JSONSerializer::SerializeTextureNode(rapidjson::Value& nodeValue,
                                          TextureNode* node) {
  Texture* texture = node->Get();
  ASSERT(texture->mTexelData);
  string b64 = base64_encode((UCHAR*)texture->mTexelData, texture->mTexelDataByteCount);
  nodeValue.AddMember("width", texture->mWidth, *mAllocator);
  nodeValue.AddMember("height", texture->mHeight, *mAllocator);
  nodeValue.AddMember("type", rapidjson::Value(
    EnumMapperA::GetStringFromEnum(TexelTypeMapper, int(texture->mType)), *mAllocator),
    *mAllocator);
  nodeValue.AddMember("base64", b64, *mAllocator);
}

void JSONSerializer::SerializeStaticMeshNode(rapidjson::Value& nodeValue,
                                             StaticMeshNode* node) {
  Mesh* mesh = node->GetMesh();
  ASSERT(mesh->mRawVertexData != nullptr);
  nodeValue.AddMember("format", mesh->mFormat->mBinaryFormat, *mAllocator);
  nodeValue.AddMember("vertexcount", mesh->mVertexCount, *mAllocator);
  nodeValue.AddMember("indexcount", mesh->mIndexCount, *mAllocator);

  UINT floatCount = mesh->mVertexCount * mesh->mFormat->mStride / sizeof(float);
  float* attribs = reinterpret_cast<float*>(mesh->mRawVertexData);
  rapidjson::Value attribArray(rapidjson::kArrayType);
  for (UINT i = 0; i < floatCount; i++) {
    attribArray.PushBack(double(attribs[i]), *mAllocator);
  }
  nodeValue.AddMember("vertices", attribArray, *mAllocator);

  if (mesh->mIndexCount > 0) {
    rapidjson::Value indexArray(rapidjson::kArrayType);
    for (UINT i = 0; i < mesh->mIndexCount; i++) {
      indexArray.PushBack(UINT(mesh->mIndexData[i]), *mAllocator);
    }
    nodeValue.AddMember("indices", indexArray, *mAllocator);
  }
}

void JSONSerializer::SerializeStubNode(rapidjson::Value& nodeValue, StubNode* node) {
  const string& f = node->mSource.Get();
  rapidjson::Value slotValue(rapidjson::kObjectType);
  nodeValue.AddMember("source", f, *mAllocator);
}


void JSONSerializer::SerializeValueSlot(rapidjson::Value& slotsObject, Slot* slot,
                                        rapidjson::Value& defaultValue) {
  rapidjson::Value slotValue(rapidjson::kObjectType);
  slotValue.AddMember("default", defaultValue, *mAllocator);
  Node* connectedNode = slot->GetReferencedNode();
  if (!slot->IsDefaulted()) {
    int connectedID = mNodes.at(connectedNode);
    slotValue.AddMember("id", connectedID, *mAllocator);
  }
  slotsObject.AddMember(rapidjson::Value(*slot->GetName(), *mAllocator),
                        slotValue, *mAllocator);
}
