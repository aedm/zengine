#include <include/nodes/fluidnode.h>
#include <include/nodes/meshgenerators.h>
#include <include/shaders/engineshaders.h>

REGISTER_NODECLASS(FluidNode, "Fluid");

const int COLOR_RESOLUTION = 1024;
const int VELOCITY_RESOLUTION = 512;
const float COLOR_PIXEL_SIZE = 1.0f / float(COLOR_RESOLUTION);
const float VELOCITY_TEXEL_SIZE = 1.0f / float(VELOCITY_RESOLUTION);

FluidNode::FluidNode()
  : Node()
  , mCurlAmount(this, "Curl", false, true, true, 0.0f, 30.0f)
  , mPressureFade(this, "Pressure Fade")
  , mVelocityDissipation(this, "Velocity Dissipation")
  , mColorDissipation(this, "Color Dissipation")
  , mIterationCount(this, "Iterations")
{
  mCurlAmount.SetDefaultValue(30.0f);
  mPressureFade.SetDefaultValue(0.8f);
  mVelocityDissipation.SetDefaultValue(0.5f);
  mColorDissipation.SetDefaultValue(0.5f);

  mColor1Texture = OpenGL->MakeTexture(COLOR_RESOLUTION, COLOR_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mColor2Texture = OpenGL->MakeTexture(COLOR_RESOLUTION, COLOR_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mVelocity1Texture = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mVelocity2Texture = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mVelocity3Texture = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mCurlTexture = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mDivergenceTexture = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mPressureTexture1 = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mPressureTexture2 = OpenGL->MakeTexture(VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    TexelType::ARGB16F, nullptr, true, false, false, false);

  mColor1FBID = OpenGL->CreateFrameBuffer(nullptr, mColor1Texture, nullptr);
  mColor2FBID = OpenGL->CreateFrameBuffer(nullptr, mColor2Texture, nullptr);
  mVelocity1FBID = OpenGL->CreateFrameBuffer(nullptr, mVelocity1Texture, nullptr);
  mVelocity2FBID = OpenGL->CreateFrameBuffer(nullptr, mVelocity2Texture, nullptr);
  mVelocity3FBID = OpenGL->CreateFrameBuffer(nullptr, mVelocity3Texture, nullptr);
  mCurlFBID = OpenGL->CreateFrameBuffer(nullptr, mCurlTexture, nullptr);
  mDivergenceFBID = OpenGL->CreateFrameBuffer(nullptr, mDivergenceTexture, nullptr);
  mPressure1FBID = OpenGL->CreateFrameBuffer(nullptr, mPressureTexture1, nullptr);
  mPressure2FBID = OpenGL->CreateFrameBuffer(nullptr, mPressureTexture2, nullptr);
}


FluidNode::~FluidNode() = default;

void FluidNode::Render(float deltaTime) const
{
  Globals globals;
  globals.FluidVelocity1 = mVelocity1Texture;
  globals.FluidVelocity2 = mVelocity2Texture;
  globals.FluidVelocity3 = mVelocity3Texture;
  globals.FluidColor = mColor1Texture;
  globals.FluidCurl = mCurlTexture;
  globals.FluidDivergence = mDivergenceTexture;
  globals.FluidPressure = mPressureTexture1;
  globals.FluidForceDamp = 0.0001f;
  globals.FluidCurlAmount = mCurlAmount.Get();
  globals.FluidPressureFade = mPressureFade.Get();
  globals.FluidDeltaTime = deltaTime;
  globals.RenderTargetSizeRecip = Vec2(VELOCITY_TEXEL_SIZE, VELOCITY_TEXEL_SIZE);

  /// Curl step
  OpenGL->SetViewport(0, 0, VELOCITY_RESOLUTION, VELOCITY_RESOLUTION);

  TheEngineShaders->mFluid_CurlPass->Set(&globals);
  OpenGL->SetFrameBuffer(mCurlFBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Vorticity step
  TheEngineShaders->mFluid_VorticityPass->Set(&globals);
  OpenGL->SetFrameBuffer(mVelocity2FBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Divergence step
  TheEngineShaders->mFluid_DivergencePass->Set(&globals);
  OpenGL->SetFrameBuffer(mDivergenceFBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Pressure fade step
  TheEngineShaders->mFluid_FadeOutPass->Set(&globals);
  OpenGL->SetFrameBuffer(mPressure2FBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Pressure iteration
  OpenGL->SetFrameBuffer(mPressure2FBID);
  TheEngineShaders->mFluid_PressurePass->Set(&globals);
  const int iterations = int(mIterationCount.Get());
  for (int i = 0; i < iterations; i++) {
    OpenGL->BlitFrameBuffer(mPressure2FBID, mPressure1FBID,
      0, 0, VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
      0, 0, VELOCITY_RESOLUTION, VELOCITY_RESOLUTION);
    TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);
  }
  OpenGL->BlitFrameBuffer(mPressure2FBID, mPressure1FBID,
    0, 0, VELOCITY_RESOLUTION, VELOCITY_RESOLUTION,
    0, 0, VELOCITY_RESOLUTION, VELOCITY_RESOLUTION);

  /// Gradient subtract
  TheEngineShaders->mFluid_GradientSubtractPass->Set(&globals);
  OpenGL->SetFrameBuffer(mVelocity3FBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Velocity advection
  globals.FluidDissipation = mVelocityDissipation.Get();
  globals.FluidColor = mVelocity3Texture;
  TheEngineShaders->mFluid_AdvectionPass->Set(&globals);
  OpenGL->SetFrameBuffer(mVelocity1FBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Color advection
  OpenGL->SetViewport(0, 0, COLOR_RESOLUTION, COLOR_RESOLUTION);
  globals.RenderTargetSizeRecip = Vec2(COLOR_PIXEL_SIZE, COLOR_PIXEL_SIZE);
  globals.FluidDissipation = mColorDissipation.Get();
  globals.FluidColor = mColor1Texture;
  TheEngineShaders->mFluid_AdvectionPass->Set(&globals);
  OpenGL->SetFrameBuffer(mColor2FBID);
  TheEngineShaders->mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  /// Blit back to color buffer #1
  OpenGL->BlitFrameBuffer(mColor2FBID, mColor1FBID,
    0, 0, COLOR_RESOLUTION, COLOR_RESOLUTION,
    0, 0, COLOR_RESOLUTION, COLOR_RESOLUTION);
}

void FluidNode::SetGlobalFluidTextures(Globals* globals) const
{
  globals->FluidColor = mColor1Texture;
  globals->FluidVelocity1 = mVelocity1Texture;
  globals->FluidVelocity2 = mVelocity2Texture;
  globals->FluidVelocity3 = mVelocity3Texture;
  globals->FluidCurl = mCurlTexture;
  globals->FluidDivergence = mDivergenceTexture;
  globals->FluidPressure = mPressureTexture1;
}

void FluidNode::SetColorRenderTarget() const
{
  OpenGL->SetFrameBuffer(mColor1FBID);
  OpenGL->SetViewport(0, 0, COLOR_RESOLUTION, COLOR_RESOLUTION);
}

void FluidNode::SetVelocityRenderTarget() const
{
  OpenGL->SetFrameBuffer(mVelocity1FBID);
  OpenGL->SetViewport(0, 0, VELOCITY_RESOLUTION, VELOCITY_RESOLUTION);
}
