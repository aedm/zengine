#pragma once

#include "propertyeditor.h"
#include <QtGui/QIcon>

class SlotWatcher;

/// Widget that displays editable node slots
class SlotEditor : public PropertyEditor {
public:
  SlotEditor(const std::shared_ptr<Node>& node);
  virtual ~SlotEditor();

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  std::map<Slot*, std::shared_ptr<SlotWatcher>> mSlotWatchers;

  void OnSlotConnectionChanged(Slot* slot) override;
  void OnSlotStructureChanged() override;

  void RemoveWatcherWidget(WatcherWidget* watcherWidget);

  /// Adds a slot to the UI. Returns false on type mismatch
  template <ValueType T>
  bool AddSlot(Slot* slot, QWidget* parent, QLayout* layout);

  void RebuildSlots();
  void RemoveAllSlots();

  QIcon mGhostIcon;

  /// This widget holds all slot widgets
  QWidget* mSlotsWidget = nullptr;
  QVBoxLayout* mSlotLayout = nullptr;
};


/// A parameter panel item for slots
class SlotWatcher : public WatcherUi {
public:
  /// Enable/disable editing
  virtual void UpdateReadOnly() = 0;

protected:
  SlotWatcher(const std::shared_ptr<Node>& node);
};


/// A parameter panel item for value slots
template <ValueType T>
class TypedSlotWatcher : public SlotWatcher {
  typedef typename ValueTypes<T>::Type Type;

public:
  TypedSlotWatcher(ValueSlot<Type>* slot);

  /// Enable/disable editing
  void UpdateReadOnly() override;

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  ValueEditor<T>* mEditor = nullptr;

  /// The slot through which the Node is inspected
  ValueSlot<Type>* mSlot = nullptr;

  /// Handles value change on UI
  void HandleValueChange(QWidget* widget, const Type& value);
};

class PassSlotEditor : public SlotEditor {
public:
  PassSlotEditor(const std::shared_ptr<Node>& node);

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;
};