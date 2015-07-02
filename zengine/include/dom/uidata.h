#pragma once

/// Data that's not necessary for the engine, but used by an UI-based editor.

#include "../base/vectormath.h"
#include <string>
#include <set>

using namespace std;

class Slot;

/// UI data for nodes
struct NodeUI {
  NodeUI();

private:
  Vec2 mPosition;
  string mNodeName;
};


/// UI data for slots
struct SlotUI {
  SlotUI();

private:
  string mName;
};

