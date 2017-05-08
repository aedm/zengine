#pragma once
/// Include file for Zengine.

#include "resources/resourcemanager.h"

#include "base/helpers.h"

#include "dom/nodetype.h"
#include "dom/node.h"
#include "dom/graph.h"
#include "dom/document.h"
#include "dom/watcher.h"

#include "resources/mesh.h"
#include "resources/texture.h"

#include "shaders/stubnode.h"
#include "shaders/shadernode.h"
#include "shaders/pass.h"
#include "shaders/material.h"
#include "shaders/enginestubs.h"
#include "shaders/engineshaders.h"

#include "render/rendertarget.h"

#include "nodes/drawable.h"
#include "nodes/valuenodes.h"
#include "nodes/meshnode.h"
#include "nodes/timenode.h"
#include "nodes/scenenode.h"
#include "nodes/clipnode.h"
#include "nodes/movienode.h"
#include "nodes/vectornodes.h"
#include "nodes/splinenode.h"
#include "nodes/cameranode.h"
#include "nodes/meshgenerators.h"

/// Inits Zengine. Returns true if everything went okay.
bool InitZengine();

/// Closes Zengine, frees up resources
void CloseZengine();

/// Will be called once after Zengine init is done
extern Event<> OnZengineInitDone;
