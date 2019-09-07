#pragma once

#include "../nodes/valuenodes.h"
#include "../resources/texture.h"

template<> StaticValueNode<shared_ptr<Texture>>::StaticValueNode();

typedef ValueNode<shared_ptr<Texture>> TextureNode;
typedef ValueSlot<shared_ptr<Texture>> TextureSlot;

typedef StaticValueNode<shared_ptr<Texture>> StaticTextureNode;

class TextureFileNode : public TextureNode {
public:
  TextureFileNode();
  StringSlot mFileName;

  virtual const shared_ptr<Texture>& Get() override;

  virtual void HandleMessage(Message* message) override;

private:
  shared_ptr<Texture> mTexture;
};