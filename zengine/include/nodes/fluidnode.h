#pragma once

#include "../dom/node.h"
#include "../resources/texture.h"
#include "texturenode.h"
#include "../shaders/stubnode.h"

class FluidNode : public Node {
public:
  FluidNode();
  ~FluidNode();

  FloatSlot mCurlAmount;
  FloatSlot mPressureFade;
  FloatSlot mVelocityDissipation;
  FloatSlot mColorDissipation;
  FloatSlot mIterationCount;

  /// steps: 
  /// - curl (velocity1 -> curl)
  /// - vorticity (velocity1 + curl -> velocity2) 
  /// - divergence (velocity2 -> divergence)
  /// - fade out pressure (pressure1 -> pressure2)
  /// - pressure iteration (divergence + pressure12 -> pressure12)
  /// - optional: blit to pressure1
  /// - gradient subtract (pressure1 + velocity2 -> velocity3)
  /// - velocity advection (velocity3 -> velocity1)
  /// - color advection (color1 + velocity1 -> color2)
  /// - color blitback (color2 -> color1)

  /// Render a fluid step
  void Render(float deltaTime);
  void SetGlobalFluidTextures(Globals* globals);
  void SetColorRenderTarget();
  void SetVelocityRenderTarget();

private:
  FrameBufferId mColor1FBID = 0;
  FrameBufferId mColor2FBID = 0;
  FrameBufferId mVelocity1FBID = 0;
  FrameBufferId mVelocity2FBID = 0;
  FrameBufferId mVelocity3FBID = 0;
  FrameBufferId mCurlFBID = 0;
  FrameBufferId mDivergenceFBID = 0;
  FrameBufferId mPressure1FBID = 0;
  FrameBufferId mPressure2FBID = 0;
  FrameBufferId mExternalRenderTarget = 0;

  shared_ptr<Texture> mColor1Texture = nullptr;
  shared_ptr<Texture> mColor2Texture = nullptr;
  shared_ptr<Texture> mVelocity1Texture = nullptr;
  shared_ptr<Texture> mVelocity2Texture = nullptr;
  shared_ptr<Texture> mVelocity3Texture = nullptr;
  shared_ptr<Texture> mCurlTexture = nullptr;
  shared_ptr<Texture> mDivergenceTexture = nullptr;
  shared_ptr<Texture> mPressureTexture1 = nullptr;
  shared_ptr<Texture> mPressureTexture2 = nullptr;

  //shared_ptr<StaticTextureNode> mColor1TextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mColor2TextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mVelocity1TextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mVelocity2TextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mVelocity3TextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mCurlTextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mDivergenceTextureNode = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mPressureTexture1Node = make_shared<StaticTextureNode>();
  //shared_ptr<StaticTextureNode> mPressureTexture2Node = make_shared<StaticTextureNode>();
};

typedef TypedSlot<FluidNode> FluidSlot;
