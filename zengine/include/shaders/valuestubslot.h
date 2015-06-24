#pragma once

#include "../nodes/valuenodes.h"

template <NodeType T>
class ValueStubSlot: public ValueSlot<T> {
public:
  ValueStubSlot(Node* owner, SharedString name);
  virtual bool DoesAcceptType(NodeType type) const override;
};


template <NodeType T>
ValueStubSlot<T>::ValueStubSlot(Node* owner, SharedString name)
  : ValueSlot(owner, name)
{}


template <NodeType T>
bool ValueStubSlot<T>::DoesAcceptType(NodeType type) const {
  return type == NodeType::SHADER_STUB || type == mType;
}
