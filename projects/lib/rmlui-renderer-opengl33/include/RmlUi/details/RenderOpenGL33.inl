#pragma once

#define RML_RENDERER_OPENGL33_CONTEXT_CHECK(ctx_type) \
    static_assert(meta::OpenGL33Context<ctx_type>, "Must contains OpenGL v3.3 Core Profile function pointers")

namespace Rml
{

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::~RendererOpenGL33()
{
}

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::RendererOpenGL33(Gl_Context_Provider get_gl)
    : get_gl_(get_gl)
{
    RML_RENDERER_OPENGL33_CONTEXT_CHECK(GL);
    if (!get_gl_)
    {
        throw std::invalid_argument("Non nullptr gl context provider should be set");
    }
}

template<meta::OpenGL33ContextForward GL>
inline CompiledGeometryHandle
RendererOpenGL33<GL>::CompileGeometry(Span<const Vertex> vertices, Span<const int> indices)
{
    auto &gl = *get_gl_();

    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseGeometry(CompiledGeometryHandle geometry)
{
    auto &gl = *get_gl_();
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::LoadTexture(Vector2i &texture_dimensions, const String &source)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::GenerateTexture(Span<const byte> source, Vector2i source_dimensions)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseTexture(TextureHandle texture)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::EnableScissorRegion(bool enable)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::SetScissorRegion(Rectanglei region)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::EnableClipMask(bool enable)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::RenderToClipMask(ClipMaskOperation operation, CompiledGeometryHandle geometry, Vector2f translation)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::SetTransform(const Matrix4f *transform)
{
}

template<meta::OpenGL33ContextForward GL>
inline LayerHandle
RendererOpenGL33<GL>::PushLayer()
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::CompositeLayers(LayerHandle source, LayerHandle destination, BlendMode blend_mode, Span<const CompiledFilterHandle> filters)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::PopLayer()
{
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::SaveLayerAsTexture()
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::SaveLayerAsMaskImage()
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::CompileFilter(const String &name, const Dictionary &parameters)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseFilter(CompiledFilterHandle filter)
{
}

template<meta::OpenGL33ContextForward GL>
inline CompiledShaderHandle
RendererOpenGL33<GL>::CompileShader(const String &name, const Dictionary &parameters)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseShader(CompiledShaderHandle shader)
{
}
} // namespace Rml
