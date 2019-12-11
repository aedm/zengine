#include <include/nodes/propertiesnode.h>

REGISTER_NODECLASS(PropertiesNode, "Properties");

PropertiesNode::PropertiesNode() 
  : mBPM(this, "BPM", false, true, true, 60, 160)
{
  mBPM.SetDefaultValue(135);
}
