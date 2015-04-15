#include "command.h"

CommandStack* TheCommandStack = new CommandStack();

Command::~Command() {}

Command::Command()
{
	Active = false;
}

CommandStack::CommandStack() {}

CommandStack::~CommandStack()
{
	for (int i = CommandList.size(); i>0;)
	{
		delete CommandList[--i];
	}
}


bool CommandStack::Execute(Command *Com)
{
	if (Com->Do())
	{
		Com->Active = true;
		CommandList.push_back(Com);
		Com->OnCommand(Com);
		return true;
	}
	else
	{
		delete Com;
		return false;
	}
}

bool CommandStack::Redo()
{
	NOT_IMPLEMENTED;
	return true;
}

bool CommandStack::Undo()
{
	NOT_IMPLEMENTED;
	return true;
}
