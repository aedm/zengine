//#include <include/nodes/texturenode.h>
//#include <include/resources/resourcemanager.h>
//
//REGISTER_NODECLASS(TextureNode, "Texture");
//
//TextureNode::TextureNode() {}
//
//TextureNode::~TextureNode() {
//  if (mTexture) TheResourceManager->DiscardTexture(mTexture);
//}
//
//Texture* TextureNode::GetTexture() const {
//  return mTexture;
//}
//
//void TextureNode::Set(OWNERSHIP Texture* texture) {
//  if (mTexture) TheResourceManager->DiscardTexture(mTexture);
//  mTexture = texture;
//  SendMsg(MessageType::VALUE_CHANGED);
//}
//
//
