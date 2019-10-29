#pragma once

#include "nodetype.h"
#include "watcher.h"
#include "../base/vectormath.h"
#include "../base/fastdelegate.h"
#include "../base/helpers.h"
#include <vector>
#include <set>
#include <memory>
#include <deque>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

using namespace fastdelegate;

class Node;
class Watcher;
class Slot;
class MessageQueue;

extern MessageQueue TheMessageQueue;


/// Notification types that nodes send to each other when something happened.
enum class MessageType {
  /// Some slots were added or removed.
  SLOT_STRUCTURE_CHANGED,

  /// Direct slot connection changed.
  SLOT_CONNECTION_CHANGED,

  /// Slot's ghost flag changed
  SLOT_GHOST_FLAG_CHANGED,

  /// The transitive closure changed
  TRANSITIVE_CLOSURE_CHANGED,

  /// Ghost flag changes somewhere down the line
  TRANSITIVE_GHOST_CHANGED,

  /// The value of a connected node changed, reevaluation might be needed
  VALUE_CHANGED,

  /// Node contents changed, watchers need to redraw it.
  NEEDS_REDRAW,

  /// Name of the node changed
  NODE_NAME_CHANGED,

  /// The node will be removed from the editor immediately. The underlying object 
  /// is not necessarily deleted, it's just removed from the document, and all of its
  /// watchers need to be deactivated. If the object stays in the command stack, the 
  /// removal might be undone later.
  /// This message is only emitted by the editor.
  NODE_REMOVED,

  /// Position of the node changed
  NODE_POSITION_CHANGED,

  /// Scene time is manually edited in a spline editor
  SCENE_TIME_EDITED,
};

struct Message {
  std::shared_ptr<Node> mSource;
  std::shared_ptr<Node> mTarget;
  Slot* mSlot;
  MessageType mType;

  bool operator < (const Message& other) const;
};


class MessageQueue {
  friend class Node;

public:
  void Enqueue(const std::shared_ptr<Node>& source, const std::shared_ptr<Node>& target, 
    MessageType type, Slot* slot = nullptr);

private:
  std::set<Message> mMessageSet;
  std::deque<Message> mMessageQueue;
  bool mIsInProgress = false;

  void ProcessAllMessages();
  void RemoveNode(Node* node);
};


/// Nodes can have multiple input slots, which connect them to other nodes' slots.
class Slot {
public:
  Slot(Node* owner, std::string name, bool isMultiSlot = false, bool isPublic = true,
    bool isSerializable = true, bool isTraversable = true);
  virtual ~Slot();

  std::shared_ptr<Node> GetOwner() const;
  bool IsOwnerExpired() const;

  /// Attaches slot to node. 
  /// - for non-multislots, this overrides the current connection.
  /// - for multislots, the node will be added to the list of connected nodes. 
  /// Returns false if connection is not possible due to type mismatch.
  virtual bool Connect(const std::shared_ptr<Node>& target);

  /// Disconnects a node from this slot. 
  virtual void Disconnect(const std::shared_ptr<Node>&);

  /// Disconnects all nodes from this slot. If NotifyOwner is true, the slot
  /// send a SLOT_CONNECTION_CHANGED message to its owner.
  virtual void DisconnectAll(bool notifyOwner);

  /// Removes the connected node from connected nodes list, 
  /// and reinserts it at the "TargetIndex" position. Only for multislots.
  void ChangeNodeIndex(const std::shared_ptr<Node>& node, UINT targetIndex);

  /// Returns referenced connected node (single slots only)
  /// Subclasses of Slot have a properly typed GetNode() method.
  std::shared_ptr<Node> GetReferencedNode() const;

  /// Returns directly connected node, no reference following (single slots only)
  std::shared_ptr<Node> GetDirectNode() const;

  /// Returns the number of connected nodes (only for multislot)
  UINT GetMultiNodeCount() const;

  /// Returns the 'index'th connected node reference (only for multislot)
  std::shared_ptr<Node> GetReferencedMultiNode(UINT index) const;

  /// Returns the multinodes *without* forwarding,
  /// ie. not calling Node::GetReferencedNode()
  const std::vector<std::shared_ptr<Node>>& GetDirectMultiNodes() const;
  
  /// Type of object this slot accepts
  virtual bool DoesAcceptNode(const std::shared_ptr<Node>& node) const;

  /// The name of the slot. Can only be set once. The reason it's not a const
  /// is that it's not necessarily known at constructor time.
  const std::string mName;

  /// True if the slot can connect to multiple nodes
  const bool mIsMultiSlot;

  /// Return the Nth connected node from a multislot
  const std::shared_ptr<Node>& operator[] (UINT index);

  /// Returns true if slot is connected to its own default node (if it has one)
  virtual bool IsDefaulted();

  /// Set ghost bit
  void SetGhost(bool isGhostSlot);
  bool IsGhost() const;

protected:
  /// The operator which this slot is a member of
  Node* const mOwner;

  /// The slot is connected to this node (nullptr if multislot)
  std::shared_ptr<Node> mNode;

  /// The slot is connected to these nodes (empty if not multislot)
  std::vector<std::shared_ptr<Node>> mMultiNodes;

  /// Ghost slots will be the slots of ghost nodes. They are free parameters 
  /// that users can change.
  bool mGhostSlot = false;
};

/// An operation that takes its slot values as input and computes an output
class Node: public std::enable_shared_from_this<Node> {
  friend class Slot;
  friend class Watcher;
  friend class MessageQueue;
  template<typename T> friend class ValueSlot;

public:
  virtual ~Node();

  /// List of slots this node's output is connected to
  const std::vector<Slot*>& GetDependants() const;

  /// Reruns Operate() if not up to date (also updates on ancestors)
  void Update();

  /// Calculates the transitive closure of the node to "oResult" in topological ordering.
  /// Deepest nodes come first. 
  void GenerateTransitiveClosure(std::vector<std::shared_ptr<Node>>& oResult, 
    bool includeHiddenSlots);

  /// Returns a node which can actually do what it claims to do. Most nodes
  /// return "this", but there are a few exceptions that refer to a different node:
  /// - Ghost nodes are references to other nodes (TODO)
  /// - Composite nodes have internal, hidden nodes that do the heavy lifting
  virtual std::shared_ptr<Node> GetReferencedNode();

  virtual bool IsGhostNode();

  /// Disconnects all outgoing connections
  void Dispose();

  /// Copies content from other node of the same type.
  /// This is useful for creating new nodes with predefined content, eg shader stubs.
  virtual void CopyFrom(const std::shared_ptr<Node>& node);

protected:
  Node(bool isForwarderNode = false);

  /// True is all slots are properly connected
  bool mIsProperlyConnected;

  /// Main operation
  virtual void Operate() {}

  /// Sends a message to dependants. ('SendMessage' is already defined in WinUser.h)
  void SendMsg(MessageType message);

  /// Handle received messages
  virtual void HandleMessage(Message* message);

  /// Check if all slots are properly connected to an operator
  void CheckConnections();

  /// True if Operate() needs to be called
  bool mIsUpToDate;

  /// Enqueues message
  void EnqueueMessage(MessageType message, Slot* slot = nullptr, 
    const std::shared_ptr<Node>& sender = nullptr);

private:
  /// Slots that this node is connected to (as an input)
  std::vector<Slot*> mDependants;

  /// Add or remove slot to/from notification list
  void ConnectToSlot(Slot* slot);
  void DisconnectFromSlot(Slot* slot);

  /// Receives message through a slot
  void ReceiveMessage(Message* message);

  /// ---------------- Editor-specific parts ----------------
  /// This section can be disabled without hurting the engine.
  /// --------------------------------------------------------
public:
  virtual void SetName(const std::string& name);
  virtual const std::string& GetName() const;
  virtual void SetPosition(vec2 position);
  virtual vec2 GetPosition() const;
  virtual void SetSize(vec2 size);
  virtual vec2 GetSize() const;

  /// Returns the list of publicly editable slots
  const std::vector<Slot*>& GetPublicSlots() const;

  /// Returns the list of all slots
  const std::vector<Slot*>& GetTraversableSlots() const;

  /// Returns the slots that need to be serialized when saving / loading
  const std::unordered_map<std::string, Slot*>& GetSerializableSlots() const;

protected:
  /// Registers a new slot
  void AddSlot(Slot* slot, bool isPublic, bool isSerializable, bool isTraversable);

  /// Removes public and serializable slots
  void ClearSlots();

private:
  /// Custom name of the node
  std::string mName;

  /// Position and size on the Graph
  vec2 mPosition;
  vec2 mSize;

  /// Public slots of this node.
  std::vector<Slot*>	mPublicSlots;

  /// All traversable slots of this node. These include hidden slots that aren't 
  /// displayed on the UI (like SceneTime slots of SplineNodes), but they don't
  /// include generated slots (like for StubNodes).
  std::vector<Slot*>	mTraversableSlots;

  /// Slots that need to be serialized when saving / loading.
  std::unordered_map<std::string, Slot*> mSerializableSlotsByName;
  
  /// ------------------ Watcher operations ------------------
  /// This section can be disabled without hurting the engine.
  /// --------------------------------------------------------
public:
  template <typename T, typename ...P>
  std::shared_ptr<T> Watch(P... args) {
    static_assert(std::is_base_of<Watcher, T>::value, "T must be a Watcher");
    std::shared_ptr<T> watcher = std::make_shared<T>(args...);
    mWatchers.insert(watcher);
    return watcher;
  }

  /// Removes a Watcher from the watchers list
  void RemoveWatcher(const std::shared_ptr<Watcher>& watcher);
  void RemoveAllWatchers();

  /// Adds a new Watcher to the watchers list
  void AssignWatcher(const std::shared_ptr<Watcher>& watcher);

protected:
  template <class ...B>
  void NotifyWatchers(void (Watcher::*M)(B...), B... args) {
    for (const auto& watcher : mWatchers) ((watcher.get())->*M)(args...);
  }

private:
  /// Watchers
  std::set<std::shared_ptr<Watcher>> mWatchers;
};

/// A slot which only accepts a certain Node class, eg TypedSlot<ValueNode<float>>
template<class N>
class TypedSlot: public Slot {
public:
  TypedSlot(Node* owner, const std::string& name, bool isMultiSlot = false,
            bool isPublic = true, bool isSerializable = true)
    : Slot(owner, name, isMultiSlot, isPublic, isSerializable) {}

  std::shared_ptr<N> GetNode() {
    return PointerCast<N>(GetReferencedNode());
  }

  bool DoesAcceptNode(const std::shared_ptr<Node>& node) const override {
    return IsPointerOf<N>(node->GetReferencedNode());
  }
};

