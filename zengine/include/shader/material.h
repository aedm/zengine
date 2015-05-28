#pragma once

#include "pass.h"
#include "../shaders/shaders.h"

class Material: public Node
{
public:
	Material();
	virtual ~Material();

	Slot					SolidPass;

	Pass*					GetPass();
	
protected:
	virtual void			HandleMessage(Slot* S, NodeMessage Message, const void* Payload);

};