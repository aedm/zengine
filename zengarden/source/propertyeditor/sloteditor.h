#pragma once

#include "propertyeditor.h"
#include <QtGui/QIcon>

class SlotWatcher;

/// Widget that displays editable node slots
class SlotEditor : public PropertyEditor {
public:
  SlotEditor(Node* node);
  virtual ~SlotEditor();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  map<Slot*, shared_ptr<SlotWatcher>> mSlotWatchers;

  virtual void OnSlotConnectionChanged(Slot* slot) override;

  void RemoveWatcherWidget(WatcherWidget* watcherWidget);

  /// Adds a slot to the UI. Returns false on type mismatch
  template <ValueType T>
  bool AddSlot(Slot* slot, QWidget* parent, QLayout* layout);

  QIcon mGhostIcon;
};


/// A parameter panel item for slots
class SlotWatcher : public WatcherUI {
public:
  /// Enable/disable editing
  virtual void UpdateReadOnly() = 0;

protected:
  SlotWatcher(Node* node);
};


/// A parameter panel item for value slots
template <ValueType T>
class TypedSlotWatcher : public SlotWatcher {
public:
  TypedSlotWatcher(ValueSlot<T>* slot);

  /// Enable/disable editing
  virtual void UpdateReadOnly() override;

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  typedef typename ValueTypes<T>::Type Type;

  ValueEditor<T>* mEditor = nullptr;

  /// The slot through which the Node is inspected
  ValueSlot<T>* mSlot = nullptr;

  /// Handles value change on UI
  void HandleValueChange(QWidget* widget, const Type& value);
};
