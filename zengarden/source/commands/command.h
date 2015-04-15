#pragma once

#include <zengine.h>
#include <vector>

using namespace std;

class CommandStack;
extern CommandStack* TheCommandStack;

/// Command design pattern, abstract ancestor
class Command
{
	friend class CommandStack;

public:
	Command();
	virtual ~Command();

	virtual bool			Do() = NULL;
	virtual bool			Undo() = NULL;

	/// Fired after Do() and Undo()
	Event<Command*>			OnCommand;

protected:
	/// True if last action was Do(), false if Undo()
	bool					Active;
};


/// Stack of commands, for undoing
class CommandStack
{
public:
	CommandStack();
	~CommandStack();

	/// Adds the command to the stack if it can be executed
	bool Execute(Command* Com);

	/// Undo command
	bool Undo();

	/// Redo command
	bool Redo();

private:
	vector<Command*>		CommandList;
};
