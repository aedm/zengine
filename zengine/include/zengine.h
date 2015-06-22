#pragma once
/// Include file for Zengine.

#include "resources/resourcemanager.h"

#include "base/helpers.h"

#include "dom/node.h"

#include "resources/mesh.h"
#include "resources/texture.h"

#include "shaders/shaderstub.h"
#include "shaders/shadersource2.h"
#include "shaders/pass.h"
#include "shaders/material.h"
#include "shaders/drawable.h"

#include "nodes/valuenodes.h"
#include "nodes/meshnode.h"

/// Inits Zengine. Returns true if everything went okay.
bool InitZengine();

/// Closes Zengine, frees up resources
void CloseZengine();

/// Will be called once after Zengine init is done
extern Event<> OnZengineInitDone;
