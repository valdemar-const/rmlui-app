#pragma once

#include <GLFW/glfw3.h>

// Forward declarations
struct GladGLContext;

namespace Rml
{
class Context;
} // namespace Rml

namespace skif::rmlui
{

class InputManagerImpl;

/// Единый контекст окна для GLFW user pointer.
/// Агрегирует все указатели, нужные в GLFW callbacks.
struct WindowContext
{
    GladGLContext    *gl            = nullptr;
    Rml::Context     *rml_context   = nullptr;
    InputManagerImpl *input_manager = nullptr;
};

} // namespace skif::rmlui
