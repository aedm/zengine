#pragma once

#include "../dom/node.h"
#include "../resources/texture.h"
#include "../shaders/stubnode.h"

class FluidNode : public Node {
public:
  FluidNode();

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
  /// - color blit back (color2 -> color1)

  /// Render a fluid step
  void Render(float deltaTime) const;
  void SetGlobalFluidTextures(Globals* globals) const;
  void SetColorRenderTarget() const;
  void SetVelocityRenderTarget() const;

private:
  FrameBufferId mColor1FbId = 0;
  FrameBufferId mColor2FbId = 0;
  FrameBufferId mVelocity1FbId = 0;
  FrameBufferId mVelocity2FbId = 0;
  FrameBufferId mVelocity3FbId = 0;
  FrameBufferId mCurlFbId = 0;
  FrameBufferId mDivergenceFbId = 0;
  FrameBufferId mPressure1FbId = 0;
  FrameBufferId mPressure2FbId = 0;

  std::shared_ptr<Texture> mColor1Texture = nullptr;
  std::shared_ptr<Texture> mColor2Texture = nullptr;
  std::shared_ptr<Texture> mVelocity1Texture = nullptr;
  std::shared_ptr<Texture> mVelocity2Texture = nullptr;
  std::shared_ptr<Texture> mVelocity3Texture = nullptr;
  std::shared_ptr<Texture> mCurlTexture = nullptr;
  std::shared_ptr<Texture> mDivergenceTexture = nullptr;
  std::shared_ptr<Texture> mPressureTexture1 = nullptr;
  std::shared_ptr<Texture> mPressureTexture2 = nullptr;
};

typedef TypedSlot<FluidNode> FluidSlot;
