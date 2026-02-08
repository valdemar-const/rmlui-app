#pragma once

#include <RmlUi/RenderOpenGL33.hpp>

struct GladGLContext;

namespace Rml
{
using RendererGlad33 = Rml::RendererOpenGL33<GladGLContext>;
extern template class RendererOpenGL33<GladGLContext>;
} // namespace Rml
