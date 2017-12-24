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

using namespace std;
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

  /// THe transitive closure changed
  TRANSITIVE_CLOSURE_CHANGED,

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
  Node* mSource;
  Node* mTarget;
  Slot* mSlot;
  MessageType mType;

  bool operator < (const Message& other) const;
};


class MessageQueue {
  friend class Node;

public:
  void Enqueue(Node* source, Node* target, MessageType type, Slot* slot = nullptr);

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
  Slot(Node* owner, SharedString name, bool isMultiSlot = false,
       bool isPublic = true, bool isSerializable = true, bool isTraversable = true);
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

  /// Returns referenced connected node (single slots only)
  /// Subclasses of Slot have a properly typed GetNode() method.
  Node* GetReferencedNode() const;

  /// Returns directly connected node, no reference following (single slots only)
  Node* GetDirectNode() const;

  /// Returns the number of connected nodes (only for multislot)
  UINT GetMultiNodeCount() const;

  /// Returns the 'index'th connected node reference (only for multislot)
  Node* GetReferencedMultiNode(UINT index) const;

  /// Returns the multinodes *without* forwarding,
  /// ie. not calling Node::GetReferencedNode()
  vector<Node*> GetDirectMultiNodes() const;
  
  /// Type of object this slot accepts
  virtual bool DoesAcceptNode(Node* node) const;
  virtual bool DoesAcceptValueNode(ValueType type) const;

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
};

/// An operation that takes its slot values as input and computes an output
class Node {
  friend class Slot;
  friend class Watcher;
  friend class MessageQueue;
  template<ValueType T> friend class ValueSlot;

public:
  virtual ~Node();

  /// List of slots this node's output is connected to
  const vector<Slot*>& GetDependants() const;

  /// Reruns Operate() if not up to date (also updates on ancestors)
  void Update();

  /// Calculates the transitive closure of the node to "oResult" in topological ordering.
  /// Deepest nodes come first. 
  void GenerateTransitiveClosure(vector<Node*>& oResultm, bool includeHiddenSlots);

  /// Returns a node which can actually do what it claims to do. Most nodes
  /// return "this", but there are a few exceptions that refer to a different node:
  /// - Ghost nodes are references to other nodes (TODO)
  /// - Composite nodes have internal, hidden nodes that do the heavy lifting
  virtual Node* GetReferencedNode();

  /// ValueNodes returns the value type, all others return or ValueType::NONE
  ValueType GetValueType() const;

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

  /// Value type. Only ValueNodes have a non-NONE value.
  ValueType mValueType = ValueType::NONE;

private:
  /// Slots that this node is connected to (as an input)
  vector<Slot*> mDependants;

  /// Add or remove slot to/from notification list
  void ConnectToSlot(Slot* slot);
  void DisconnectFromSlot(Slot* slot);

  /// Receives message through a slot
  void ReceiveMessage(Message* message);

  /// ---------------- Editor-specific parts ----------------
  /// This section can be disabled without hurting the engine.
  /// --------------------------------------------------------
public:
  virtual void SetName(const string& name);
  virtual const string& GetName() const;
  virtual void SetPosition(const Vec2 position);
  virtual const Vec2 GetPosition() const;
  virtual void SetSize(const Vec2 size);
  virtual const Vec2 GetSize() const;

  /// Returns the list of publicly editable slots
  const vector<Slot*>& GetPublicSlots();

  /// Returns the list of all slots
  const vector<Slot*>& GetTraversableSlots();

  /// Returns the slots that need to be serialized when saving / loading
  const unordered_map<SharedString, Slot*>& GetSerializableSlots();

protected:
  /// Registers a new slot
  void AddSlot(Slot* slot, bool isPublic, bool isSerializable, bool isTraversable);

  /// Removes public and serializable slots
  void ClearSlots();

private:
  /// Custom name of the node
  string mName;

  /// Position and size on the Graph
  Vec2 mPosition;
  Vec2 mSize;

  /// Public slots of this node.
  vector<Slot*>	mPublicSlots;

  /// All traversable slots of this node. These include hidden slots that aren't 
  /// displayed on the UI (like SceneTime slots of SplineNodes), but they don't
  /// include generated slots (like for StubNodes).
  vector<Slot*>	mTraversableSlots;

  /// Slots that need to be serialized when saving / loading.
  unordered_map<SharedString, Slot*> mSerializableSlotsByName;
  
  /// ------------------ Watcher operations ------------------
  /// This section can be disabled without hurting the engine.
  /// --------------------------------------------------------
public:
  template <typename T, typename ...P>
  inline shared_ptr<T> Watch(P... args) {
    static_assert(std::is_base_of<Watcher, T>::value, "T must be a Watcher");
    shared_ptr<T> watcher = make_shared<T>(args...);
    mWatchers.insert(watcher);
    return watcher;
  }

  /// Removes a Watcher from the watchers list
  void RemoveWatcher(Watcher* watcher);

  /// Adds a new Watcher to the watchers list
  void AssignWatcher(shared_ptr<Watcher> watcher);

protected:
  template <class ...B>
  inline void NotifyWatchers(void (Watcher::*M)(B...), B... args) {
    for (auto watcher : mWatchers) ((watcher.get())->*M)(args...);
  }

private:
  /// Watchers
  set<shared_ptr<Watcher>> mWatchers;
};

template <ValueType T> class ValueNode;

/// Typed slot
template<class N>
class TypedSlot: public Slot {
public:
  TypedSlot(Node* owner, SharedString name, bool isMultiSlot = false,
            bool isPublic = true, bool isSerializable = true)
    : Slot(owner, name, isMultiSlot, isPublic, isSerializable) {}

  N* GetNode() {
    return SafeCastAllowNull<N*>(GetReferencedNode());
  }

  virtual bool DoesAcceptNode(Node* node) const override {
    return IsInsanceOf<N*>(node->GetReferencedNode());
  }

  virtual bool DoesAcceptValueNode(ValueType type) const override {
    return false;
  }
};
