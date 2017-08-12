#pragma once

#include "meshnode.h"

class CubeMeshNode: public MeshNode {
public:
  CubeMeshNode();

protected:
  FloatSlot mSizeX;
  FloatSlot mSizeY;
  FloatSlot mSizeZ;

  virtual void Operate() override;

  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot) override;
};