#pragma once

namespace Rml
{

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::Impl::~Impl() = default;

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::Impl::Impl(Gl_Context_Provider get_gl)
    : get_gl_(get_gl)
{
    if (!get_gl_)
    {
        throw std::invalid_argument("Non nullptr gl context provider should be set");
    }
}

template<meta::OpenGL33ContextForward GL>
inline CompiledGeometryHandle
RendererOpenGL33<GL>::Impl::CompileGeometry(Span<const Vertex> vertices, Span<const int> indices)
{
    auto &gl = *get_gl_();

    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseGeometry(CompiledGeometryHandle geometry)
{
    auto &gl = *get_gl_();
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::LoadTexture(Vector2i &texture_dimensions, const String &source)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::GenerateTexture(Span<const byte> source, Vector2i source_dimensions)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseTexture(TextureHandle texture)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::EnableScissorRegion(bool enable)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::SetScissorRegion(Rectanglei region)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::EnableClipMask(bool enable)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::RenderToClipMask(ClipMaskOperation operation, CompiledGeometryHandle geometry, Vector2f translation)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::SetTransform(const Matrix4f *transform)
{
}

template<meta::OpenGL33ContextForward GL>
inline LayerHandle
RendererOpenGL33<GL>::Impl::PushLayer()
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::CompositeLayers(LayerHandle source, LayerHandle destination, BlendMode blend_mode, Span<const CompiledFilterHandle> filters)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::PopLayer()
{
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::SaveLayerAsTexture()
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::Impl::SaveLayerAsMaskImage()
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::Impl::CompileFilter(const String &name, const Dictionary &parameters)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseFilter(CompiledFilterHandle filter)
{
}

template<meta::OpenGL33ContextForward GL>
inline CompiledShaderHandle
RendererOpenGL33<GL>::Impl::CompileShader(const String &name, const Dictionary &parameters)
{
    return 0;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseShader(CompiledShaderHandle shader)
{
}
} // namespace Rml
