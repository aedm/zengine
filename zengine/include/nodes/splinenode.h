#pragma once

//#include "../base/vectormath.h"
#include "valuenodes.h"
#include "timenode.h"
#include <vector>

using namespace std;

struct SSplinePoint
{
	SSplinePoint();

	float			time;
	float			value;
	float			tangentBefore, tangentAfter;

  // Is next section is linear or curved
  bool isLinear;							

  // If true, tangents are automatically calculated
	bool isAutoangent;					

  // Indicates breakpoint (tangents are not continuous)
	bool isBreakpoint;						

	void setValue(float time, float value);
};


class SSpline: public ValueNode<NodeType::FLOAT>
{
public:
	SSpline();
  virtual ~SSpline();

  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;

  FloatSlot mTimeSlot;

  virtual const float& Get() override;

	float getValue (float time);

  UINT getNumPoints();
	const SSplinePoint& getPoint(int index);

	int addPoint (float time, float value);	
	void removePoint (int index);	

	void setPointValue (int index, float time, float value);
	void setBreakpoint(int index, bool breakpoint);
	void setLinear(int index, bool linear);
	void setAutotangent(int index, bool autotangent);

  /// Calculates tangents of the Nth control point
	void calculateTangent(int index);					

protected:
  /// Control points of spline
	vector<SSplinePoint>	points;					

  /// Default value of an empty spline
	float	defaultValue = 0.0f;	

  /// Current value
  float currentValue;

  /// Last queried point (cache)
	int	lastIndex = 0;

  /// Scene time node
  SceneTimeNode mSceneTimeNode;

  void InvalidateCurrentValue();
  virtual void Operate();
};


