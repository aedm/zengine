#include <include/nodes/texturenode.h>

REGISTER_NODECLASS(TextureNode, "Texture");

template<> StaticValueNode<shared_ptr<Texture>>::StaticValueNode()
  : ValueNode<shared_ptr<Texture>>() {
}

template<>
void StaticValueNode<shared_ptr<Texture>>::Set(const shared_ptr<Texture>& newValue) {
  mValue = newValue;
  SendMsg(MessageType::VALUE_CHANGED);
}

template class StaticValueNode<shared_ptr<Texture>>;