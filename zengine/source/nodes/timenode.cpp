#include <include/nodes/timenode.h>

REGISTER_NODECLASS(TimeNode);

Event<float> TimeNode::OnTimeChanged;


TimeNode::TimeNode() 
  : FloatNode()
{
  OnTimeChanged += Delegate(this, &TimeNode::HandleTimeChange);
}


TimeNode::~TimeNode() {
  OnTimeChanged -= Delegate(this, &TimeNode::HandleTimeChange);
}


void TimeNode::HandleTimeChange(float milliseconds) {
  Set(milliseconds);
}
