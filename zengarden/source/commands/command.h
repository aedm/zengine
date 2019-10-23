#pragma once

#include <zengine.h>
#include <vector>

class CommandStack;
extern CommandStack* TheCommandStack;

/// Command design pattern, abstract ancestor
class Command {
  friend class CommandStack;

public:
  Command();
  virtual ~Command();

  virtual bool Do() = 0;
  virtual bool Undo() = 0;

  /// Fired after Do() and Undo()
  Event<Command*> OnCommand;

protected:
  /// True if last action was Do(), false if Undo()
  bool mIsActive;
};


/// Stack of commands, for undoing
class CommandStack {
public:
  CommandStack();
  ~CommandStack();

  /// Adds the command to the stack if it can be executed
  bool Execute(Command* command);

  /// Undo command
  bool Undo();

  /// Redo command
  bool Redo();

private:
  std::vector<Command*> mCommandList;
};
