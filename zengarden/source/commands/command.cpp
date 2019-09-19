#include "command.h"

CommandStack* TheCommandStack = new CommandStack();

Command::~Command() = default;

Command::Command() {
  mIsActive = false;
}

CommandStack::CommandStack() = default;

CommandStack::~CommandStack() {
  for (int i = mCommandList.size(); i > 0;) {
    delete mCommandList[--i];
  }
}


bool CommandStack::Execute(Command* command) {
  if (command->Do()) {
    command->mIsActive = true;
    mCommandList.push_back(command);
    command->OnCommand(command);
    return true;
  } else {
    delete command;
    return false;
  }
}

bool CommandStack::Redo() {
  NOT_IMPLEMENTED;
  return true;
}

bool CommandStack::Undo() {
  NOT_IMPLEMENTED;
  return true;
}
