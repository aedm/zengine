#pragma once

#include "meshnode.h"

class CubeMeshNode: public MeshNode {
public:
  CubeMeshNode();

protected:
  FloatSlot mSizeX;
  FloatSlot mSizeY;
  FloatSlot mSizeZ;

  void Operate() override;

  /// Handle received messages
  void HandleMessage(Message* message) override;
};


class HalfCubeMeshNode : public MeshNode {
public:
  HalfCubeMeshNode();

protected:
  void Operate() override;

  /// Handle received messages
  void HandleMessage(Message* message) override;
};

class GeosphereMeshNode : public MeshNode {
public:
  GeosphereMeshNode();

  FloatSlot mResolution;
  FloatSlot mSize;
  FloatSlot mFlatten;

protected:
  void Operate() override;

  /// Handle received messages
  void HandleMessage(Message* message) override;
};

class PlaneMeshNode : public MeshNode {
public:
  PlaneMeshNode();

  FloatSlot mResolution;
  FloatSlot mSize;

protected:
  void Operate() override;

  /// Handle received messages
  void HandleMessage(Message* message) override;
};

class PolarSphereMeshNode : public MeshNode {
public:
  PolarSphereMeshNode();

  FloatSlot mResolution;
  FloatSlot mSize;

protected:
  void Operate() override;

  /// Handle received messages
  void HandleMessage(Message* message) override;
};

