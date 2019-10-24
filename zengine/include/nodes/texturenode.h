#pragma once

#include "../nodes/valuenodes.h"
#include "../resources/texture.h"

template<> StaticValueNode<std::shared_ptr<Texture>>::StaticValueNode();

typedef ValueNode<std::shared_ptr<Texture>> TextureNode;
typedef ValueSlot<std::shared_ptr<Texture>> TextureSlot;

typedef StaticValueNode<std::shared_ptr<Texture>> StaticTextureNode;

class TextureFileNode : public TextureNode {
public:
  TextureFileNode();
  StringSlot mFileName;

  const std::shared_ptr<Texture>& Get() override;

  void HandleMessage(Message* message) override;

private:
  std::shared_ptr<Texture> mTexture;
};