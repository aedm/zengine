#include <include/nodes/propertiesnode.h>

REGISTER_NODECLASS(PropertiesNode, "Properties");

static SharedString BPMSlotName = make_shared<string>("BPM");

PropertiesNode::PropertiesNode() 
  : Node(NodeType::PROPERTIES)
  , mBPM(this, BPMSlotName, false, true, true, 60, 160) 
{
  mBPM.SetDefaultValue(135);
}
