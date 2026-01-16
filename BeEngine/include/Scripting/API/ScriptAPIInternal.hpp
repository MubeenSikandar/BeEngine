// include/Scripting/API/ScriptAPIInternal.hpp
#pragma once

#include <Scripting/API/BE_SCRIPT_API.h>

namespace BeEngine {

class Scene;
class SceneManager;

namespace ScriptAPI {

/// Initialize the Script API (call from Application)
void Initialize(SceneManager *sceneManager);

/// Shutdown the Script API
void Shutdown();

/// Set the active scene
void SetActiveScene(Scene *scene);

/// Get the active scene
Scene *GetActiveScene();

/// Update all registered scripts (call each frame)
void UpdateScripts(float deltaTime);

/// Fixed update all registered scripts (call from physics loop)
void FixedUpdateScripts(float fixedDelta);

/// Late update all registered scripts
void LateUpdateScripts(float deltaTime);

} // namespace ScriptAPI
} // namespace BeEngine
