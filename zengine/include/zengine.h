#pragma once
/// Include file for Zengine.

#include "resources/resourcemanager.h"

#include "base/helpers.h"

#include "dom/nodetype.h"
#include "dom/node.h"
#include "dom/graph.h"
#include "dom/document.h"

#include "resources/mesh.h"
#include "resources/texture.h"

#include "shaders/stubnode.h"
#include "shaders/shadernode.h"
#include "shaders/pass.h"
#include "shaders/material.h"

#include "nodes/drawable.h"
#include "nodes/valuenodes.h"
#include "nodes/meshnode.h"
#include "nodes/timenode.h"
#include "nodes/scenenode.h"

/// Inits Zengine. Returns true if everything went okay.
bool InitZengine();

/// Closes Zengine, frees up resources
void CloseZengine();

/// Will be called once after Zengine init is done
extern Event<> OnZengineInitDone;
