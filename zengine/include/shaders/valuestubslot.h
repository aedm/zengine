#pragma once

#include "../nodes/valuenodes.h"
#include "./valuetype.h"

template <typename T>
class ValueStubSlot: public ValueSlot<T> {
public:
  ValueStubSlot(Node* owner, SharedString name);
  virtual bool DoesAcceptNode(const shared_ptr<Node>& node) const override;
};


template <typename T>
ValueStubSlot<T>::ValueStubSlot(Node* owner, SharedString name)
  : ValueSlot(owner, name)
{}


template <typename T>
bool ValueStubSlot<T>::DoesAcceptNode(const shared_ptr<Node>& node) const {
  return IsPointerOf<StubNode>(node) || IsPointerOf<ValueNode<T>>(node);
}
