#include <include/nodes/texturenode.h>
#include <include/serialize/imageloader.h>

REGISTER_NODECLASS(StaticTextureNode, "Texture");
REGISTER_NODECLASS(TextureFileNode, "Texture file");

template<> StaticValueNode<shared_ptr<Texture>>::StaticValueNode()
  : ValueNode<shared_ptr<Texture>>() {
}

template<>
void StaticValueNode<shared_ptr<Texture>>::Set(const shared_ptr<Texture>& newValue) {
  mValue = newValue;
  SendMsg(MessageType::VALUE_CHANGED);
}

template class StaticValueNode<shared_ptr<Texture>>;

TextureFileNode::TextureFileNode()
  : mFileName(this, "FileName", false, false, true)
{}

const std::shared_ptr<Texture>& TextureFileNode::Get() {
  return mTexture;
}

void TextureFileNode::HandleMessage(Message* message) {
  switch (message->mType) {
  case MessageType::VALUE_CHANGED:
  {
    ASSERT(message->mSlot == &mFileName);
    mTexture = Zengine::LoadTextureFromFile(Convert::StringToWstring(mFileName.Get()));
    EnqueueMessage(MessageType::NEEDS_REDRAW);
  }
  break;
  default: break;
  }
}
