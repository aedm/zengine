#pragma once

#include <memory>

class Node;
class Slot;

class Watcher {
  friend class Node;

public:
  virtual ~Watcher();

  /// Returns the live node being watched
  std::shared_ptr<Node> GetNode() const;

  /// Returns the direct node being watched
  std::shared_ptr<Node> GetDirectNode() const;

  /// Destroys the watcher and its UI elements
  virtual void OnRemovedFromNode();

  /// Called when the watcher needs to be rerendered
  virtual void OnRedraw();

  /// Called when the name of the node is changed
  virtual void OnNameChange();

  /// Called when a slot's ghost flag changed
  virtual void OnSlotGhostChange(Slot* slot);

  /// Called when the name of the node is changed
  virtual void OnChildNameChange();

  /// Called when slot connections change
  virtual void OnSlotConnectionChanged(Slot* slot);
  
  /// Called when the list of slots changed
  virtual void OnSlotStructureChanged();

  /// Called when position on graph changed
  virtual void OnGraphPositionChanged();

  /// Splines only: control points changed
  virtual void OnSplineControlPointsChanged();
  
  /// Splines only: input time changes
  virtual void OnTimeEdited(float time);

protected:
  Watcher(std::shared_ptr<Node> node);

private:
  /// Changes the node being watched. Only a Node should call this.
  void ChangeNode(const std::shared_ptr<Node>& node);

  std::shared_ptr<Node> mNode;
};