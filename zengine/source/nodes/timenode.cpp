#include <include/nodes/timenode.h>

REGISTER_NODECLASS(SceneTimeNode, "Scene Time");
REGISTER_NODECLASS(GlobalTimeNode, "Global Time");

Event<float> GlobalTimeNode::OnTimeChanged;

GlobalTimeNode::GlobalTimeNode()
  : FloatNode() 
{
  OnTimeChanged += Delegate(this, &GlobalTimeNode::HandleTimeChange);
}

GlobalTimeNode::~GlobalTimeNode() {
  OnTimeChanged -= Delegate(this, &GlobalTimeNode::HandleTimeChange);
}

void GlobalTimeNode::HandleTimeChange(float milliseconds) {
  Set(milliseconds);
}

SceneTimeNode::SceneTimeNode()
  : FloatNode() 
{}
