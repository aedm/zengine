#pragma once

#include "../nodes/valuenodes.h"
#include "../resources/texture.h"

template<> StaticValueNode<shared_ptr<Texture>>::StaticValueNode();

typedef StaticValueNode<shared_ptr<Texture>> TextureNode;
typedef ValueSlot<shared_ptr<Texture>> TextureSlot;
