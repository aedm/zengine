#pragma once

#include "../nodes/valuenodes.h"

template <ValueType T>
class ValueStubSlot: public ValueSlot<T> {
public:
  ValueStubSlot(Node* owner, SharedString name);
  virtual bool DoesAcceptNode(Node* node) const override;
};


template <ValueType T>
ValueStubSlot<T>::ValueStubSlot(Node* owner, SharedString name)
  : ValueSlot(owner, name)
{}


template <ValueType T>
bool ValueStubSlot<T>::DoesAcceptNode(Node* node) const {
  return IsInsanceOf<StubNode*>(node) || IsInsanceOf<ValueNode<T>*>(node);
}
