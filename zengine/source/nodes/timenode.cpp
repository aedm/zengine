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

void GlobalTimeNode::HandleTimeChange(float beats) {
  Set(beats);
}

SceneTimeNode::SceneTimeNode()
  : FloatNode() 
{}

void SceneTimeNode::EditTime(float time) {
  if (time == mValue) return;
  mValue = time;
  SendMsg(MessageType::SCENE_TIME_EDITED);
  SendMsg(MessageType::VALUE_CHANGED);
}
