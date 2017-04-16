#pragma once

class Node;
class Slot;

class Watcher {
  friend class Node;

public:
  virtual ~Watcher();

  /// Returns the node being watched
  Node*	GetNode();

  /// Destroys the watcher and its UI elements
  virtual void Unwatch();

  /// Called when the watcher needs to be rerendered
  virtual void OnRedraw();

  /// Called when the name of the node is changed
  virtual void OnNameChange();

  /// Called when slot connections change
  virtual void OnSlotConnectionChanged(Slot* slot);
  
  /// Called when the list of slots changed
  virtual void OnSlotStructureChanged();

  /// Called when position on graph changed
  virtual void OnGraphPositionChanged();

  /// Splines only: control points changed
  virtual void OnSplineControlPointsChanged();
  
  /// Splines only: input time changes
  virtual void OnSplineTimeChanged();

protected:
  /// Sets the node being watched to nullptr and redraws the UI.
  void ResetNode();

  Watcher(Node* node);

  Node* mNode = nullptr;

private:
  /// Changes the node being watched. Only a Node should call this.
  void ChangeNode(Node* node);
};