#pragma once

class Node;
class Slot;

class Watcher {
public:
  virtual ~Watcher();

  /// Changes the node being watched
  virtual void ChangeNode(Node* node);

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
  Watcher(Node* node);

  Node* mNode;
};