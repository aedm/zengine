#pragma once
/// Include file for Zengine.

#include "resources/resourcemanager.h"

#include "base/helpers.h"

#include "dom/node.h"

#include "resources/mesh.h"
#include "resources/texture.h"

#include "shaders/shaders.h"

#include "nodes/valuenodes.h"
#include "operators/shaderoperator.h"
#include "operators/model.h"

#include "shaders/shadermetadata.h"
#include "shaders/shaderbuilder.h"

/// Inits Zengine. Returns true if everything went okay.
bool InitZengine();

/// Closes Zengine, frees up resources
void CloseZengine();

/// Will be called once after Zengine init is done
extern Event<void> EventZengineInitDone;
