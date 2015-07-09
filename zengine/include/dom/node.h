#pragma once

#include "nodetype.h"
#include "../base/vectormath.h"
#include "../base/fastdelegate.h"
#include "../base/helpers.h"
#include <vector>
#include <memory>

using namespace std;
using namespace fastdelegate;

/// Notifications that nodes send to each other when something changed.
/// An accompanying UINT data can also be set for each event type.
enum class NodeMessage {
  /// A slot was added or removed.
  SLOT_STRUCTURE_CHANGED,

  /// Direct slot connection changed.
  SLOT_CONNECTION_CHANGED,

  /// Some transitive connection changed
  TRANSITIVE_CONNECTION_CHANGED,

  /// The value of a connected node changed, reevaluation might be needed
  VALUE_CHANGED,

  /// Node looks changed, watchers need to redraw it.
  NEEDS_REDRAW,

  /// Name of the node changed
  NODE_NAME_CHANGED,

  /// Position of the node changed
  NODE_POSITION_CHANGED,
};

class Node;

/// Nodes can have multiple input slots, which connects it to other slots.
class Slot {
public:
  Slot(NodeType type, Node* owner, SharedString name, bool isMultiSlot = false,
       bool isPublic = true, bool isSerializable = true);
  virtual ~Slot();

  /// The operator which this slot is a member of
  Node* const mOwner;

  /// Attaches slot to node. 
  /// - for non-multislots, this overrides the current connection.
  /// - for multislots, the node will be added to the list of connected nodes. 
  /// Returns false if connection is not possible due to type mismatch.
  virtual bool Connect(Node* node);

  /// Disconnects a node from this slot. 
  virtual void Disconnect(Node* node);

  /// Disconnects all nodes from this slot. If NotifyOwner is true, the slot
  /// send a SLOT_CONNECTION_CHANGED message to its owner.
  virtual void DisconnectAll(bool notifyOwner);

  /// Removes the connected node from connected nodes list, 
  /// and reinserts it at the "TargetIndex" position. Only for multislots.
  void ChangeNodeIndex(Node* node, UINT targetIndex);

  /// Returns connected node (errorlog & nullptr if multislot)
  /// Subclasses of Slot have a properly typed GetNode() method.
  Node* GetAbstractNode() const;

  /// Returns all connected nodes (only for multislot)
  const vector<Node*>& GetMultiNodes() const;

  /// Type of object this slot accepts
  virtual bool DoesAcceptType(NodeType type) const;

  /// Returns the name of the slot
  SharedString GetName();

  /// True if the slot can connect to multiple nodes
  const bool mIsMultiSlot;

  /// Return the Nth connected node from a multislot
  Node* operator[] (UINT index);

  /// Returns true if slot is connected to its own default node (if it has one)
  virtual bool IsDefaulted();

protected:
  /// The slot is connected to this node (nullptr if multislot)
  Node* mNode;

  /// The slot is connected to these nodes (empty if not multislot)
  vector<Node*> mMultiNodes;

  /// Name of the slot. Can't be changed.
  SharedString mName;

  /// Output type
  const NodeType mType;
};


/// An operation that takes its slot values as input and computes an output
class Node {
  friend class Slot;
  template<NodeType T> friend class ValueSlot;

public:
  virtual ~Node();

  /// Returns object type.
  NodeType GetType() const;

  /// List of slots this node's output is connected to
  const vector<Slot*>& GetDependants() const;

  /// Reruns Operate() if dirty (on dirty ancestors too)
  void Evaluate();

  /// Clone node
  virtual Node* Clone() const;

  /// Hook for watchers (UI only)
  Event<Slot*, NodeMessage, const void*> onMessageReceived;

protected:
  Node(NodeType type);

  /// For cloning
  Node(const Node& original);

  /// True is all slots are properly connected
  bool mIsProperlyConnected;

  /// Main operation
  virtual void Operate() {}

  /// Sends a message to dependants. ('SendMessage' is already defined in WinUser.h)
  void SendMsg(NodeMessage message, const void* payload = nullptr);

  /// Handle received messages
  virtual void HandleMessage(Slot* slot, NodeMessage message, const void* payload);

  /// Receives message through a slot
  void ReceiveMessage(Slot* slot, NodeMessage message, const void* payload = nullptr);

  /// Output type
  NodeType mType;

  /// Check if all slots are properly connected to an operator
  void CheckConnections();

private:
  /// True if Operate() needs to be called
  bool mIsDirty;

  /// Slots that this node is connected to (as an input)
  vector<Slot*> mDependants;

  /// Add or remove slot to/from notification list
  void ConnectToSlot(Slot* slot);
  void DisconnectFromSlot(Slot* slot);


/// ---------------- Editor-specific parts ----------------
/// This section can be disabled without hurting the engine.
public: 
  virtual void SetName(const string& name);
  virtual const string& GetName() const;
  virtual void SetPosition(const Vec2 position);
  virtual const Vec2 GetPosition() const;
  virtual void SetSize(const Vec2 size);
  virtual const Vec2 GetSize() const;

  /// Returns the list of publicly editable slots
  const vector<Slot*>& GetPublicSlots();

  /// Returns the slots that need to be serialized when saving / loading
  const unordered_map<SharedString, Slot*>& GetSerializableSlots();

protected:
  /// Registers a new slot
  void AddSlot(Slot* slot, bool isPublic, bool isSerializable);
  
  /// Removes public and serializable slots
  void ClearSlots();

private:
  /// Custom name of the node
  string mName;

  /// Position and size on the Graph
  Vec2 mPosition;
  Vec2 mSize;

  /// Public slots of this node. The user may
  vector<Slot*>	mPublicSlots;

  /// Slots that need to be serialized when saving / loading.
  unordered_map<SharedString, Slot*> mSerializableSlotsByName;
};



/// Typed slot macro, syntactic sugar. Unfortunately template parameters can't
/// be used as method name parts, so this is a macro instead.
/// Use it like: typedef TypedSlot<NodeType::MESH, MeshNode> MeshSlot;
/// Then: MeshNode* node = meshSlot->GetMeshNode();
template<NodeType T, class N>
class TypedSlot: public Slot {
public:
  TypedSlot(Node* owner, SharedString name, bool isMultiSlot = false,          
            bool isPublic = true, bool isSerializable = true)
            : Slot(T, owner, name, isMultiSlot, isPublic, isSerializable) {}     

  N* GetNode() { 
    return static_cast<N*>(GetAbstractNode()); 
  } 
};
