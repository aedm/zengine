#pragma once

#include "../nodes/valuenodes.h"

template <typename T>
class ValueStubSlot: public ValueSlot<T> {
public:
  ValueStubSlot(Node* owner, const string& name);
  bool DoesAcceptNode(const shared_ptr<Node>& node) const override;
};


template <typename T>
ValueStubSlot<T>::ValueStubSlot(Node* owner, const string& name)
  : ValueSlot(owner, name)
{}


template <typename T>
bool ValueStubSlot<T>::DoesAcceptNode(const shared_ptr<Node>& node) const {
  return IsPointerOf<StubNode>(node) || IsPointerOf<ValueNode<T>>(node);
}
