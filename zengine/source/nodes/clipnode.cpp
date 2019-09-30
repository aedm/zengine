#include <include/nodes/clipnode.h>

REGISTER_NODECLASS(ClipNode, "Clip");

ClipNode::ClipNode()
  : mSceneSlot(this, "scene")
  , mStartTime(this, "Start time")
  , mLength(this, "Clip Length")
  , mTrackNumber(this, "Track")
  , mFakeStartTime(this, "Fake start time")
  , mClearColorBuffer(this, "Clear color buffer")
  , mClearDepthBuffer(this, "Clear depth buffer")
  , mCopyToSecondaryBuffer(this, "Copy to secondary buffer")
  , mTargetSquareBuffer(this, "Target square buffer")
  , mApplyPostprocessBefore(this, "After postprocess")
{
  mFakeStartTime.SetDefaultValue(-1);
}

void ClipNode::Draw(RenderTarget* renderTarget, Globals* globals, float clipTime) {
  if (mCopyToSecondaryBuffer.Get() >= 0.5f) {
    const int width = int(globals->RenderTargetSize.x);
    const int height = int(globals->RenderTargetSize.y);
    OpenGLAPI::BlitFrameBuffer(renderTarget->mGBufferId, renderTarget->mSecondaryFramebuffer,
      0, 0, width, height, 0, 0, width, height);
    OpenGL->SetFrameBuffer(renderTarget->mSecondaryFramebuffer);
    OpenGL->SetFrameBuffer(renderTarget->mGBufferId);
  }

  const bool clearColor = mClearColorBuffer.Get() >= 0.5f;
  const bool clearDepth = mClearDepthBuffer.Get() >= 0.5f;

  if (clearDepth || clearColor) {
    globals->DirectToSquare = mTargetSquareBuffer.Get();
    if (globals->DirectToSquare >= 0.5f) {
      renderTarget->SetSquareBufferAsTarget(globals);
    }
    else if (globals->DirectToScreen >= 0.5f) {
      renderTarget->SetColorBufferAsTarget(globals);
    }
    else {
      renderTarget->SetGBufferAsTarget(globals);
    }
    OpenGL->Clear(clearColor, clearDepth);
  }

  const auto& scene = mSceneSlot.GetNode();
  if (!scene) return;

  float fakeClipTime = clipTime;
  const float fakeStartTime = mFakeStartTime.Get();
  if (fakeStartTime >= 0) {
    const float startTime = mStartTime.Get();
    fakeClipTime += startTime - fakeStartTime;
  }

  scene->SetSceneTime(fakeClipTime);
  scene->Draw(renderTarget, globals);
}

void ClipNode::HandleMessage(Message* message) {
  switch (message->mType) {
  case MessageType::VALUE_CHANGED:
    SendMsg(MessageType::VALUE_CHANGED);
    break;
  case MessageType::SCENE_TIME_EDITED:
    SendMsg(MessageType::SCENE_TIME_EDITED);
  default: break;
  }
}
