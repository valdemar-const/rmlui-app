#pragma once

#include <RmlUi/details/RenderOpenGL33/Impl.hpp>

#define RML_RENDERER_OPENGL33_CONTEXT_CHECK(ctx_type) \
    static_assert(meta::OpenGL33Context<ctx_type>, "Must contains OpenGL v3.3 Core Profile function pointers")

namespace Rml
{

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::~RendererOpenGL33(void) = default;

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::RendererOpenGL33(Gl_Context_Provider get_gl)
    : pimpl_(std::make_unique<Impl>(get_gl))
{
    RML_RENDERER_OPENGL33_CONTEXT_CHECK(GL);
}

template<meta::OpenGL33ContextForward GL>
inline CompiledGeometryHandle
RendererOpenGL33<GL>::CompileGeometry(Span<const Vertex> vertices, Span<const int> indices)
{
    return pimpl_->CompileGeometry(vertices, indices);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
    pimpl_->RenderGeometry(geometry, translation, texture);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseGeometry(CompiledGeometryHandle geometry)
{
    pimpl_->ReleaseGeometry(geometry);
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::LoadTexture(Vector2i &texture_dimensions, const String &source)
{
    return pimpl_->LoadTexture(texture_dimensions, source);
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::GenerateTexture(Span<const byte> source, Vector2i source_dimensions)
{
    return pimpl_->GenerateTexture(source, source_dimensions);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseTexture(TextureHandle texture)
{
    pimpl_->ReleaseTexture(texture);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::EnableScissorRegion(bool enable)
{
    pimpl_->EnableScissorRegion(enable);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::SetScissorRegion(Rectanglei region)
{
    pimpl_->SetScissorRegion(region);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::EnableClipMask(bool enable)
{
    pimpl_->EnableClipMask(enable);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::RenderToClipMask(ClipMaskOperation operation, CompiledGeometryHandle geometry, Vector2f translation)
{
    pimpl_->RenderToClipMask(operation, geometry, translation);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::SetTransform(const Matrix4f *transform)
{
    pimpl_->SetTransform(transform);
}

template<meta::OpenGL33ContextForward GL>
inline LayerHandle
RendererOpenGL33<GL>::PushLayer()
{
    return pimpl_->PushLayer();
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::CompositeLayers(LayerHandle source, LayerHandle destination, BlendMode blend_mode, Span<const CompiledFilterHandle> filters)
{
    pimpl_->CompositeLayers(source, destination, blend_mode, filters);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::PopLayer()
{
    pimpl_->PopLayer();
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::SaveLayerAsTexture()
{
    return pimpl_->SaveLayerAsTexture();
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::SaveLayerAsMaskImage()
{
    return pimpl_->SaveLayerAsMaskImage();
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::CompileFilter(const String &name, const Dictionary &parameters)
{
    return pimpl_->CompileFilter(name, parameters);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseFilter(CompiledFilterHandle filter)
{
    pimpl_->ReleaseFilter(filter);
}

template<meta::OpenGL33ContextForward GL>
inline CompiledShaderHandle
RendererOpenGL33<GL>::CompileShader(const String &name, const Dictionary &parameters)
{
    return pimpl_->CompileShader(name, parameters);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
    pimpl_->RenderShader(shader, geometry, translation, texture);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::ReleaseShader(CompiledShaderHandle shader)
{
    pimpl_->ReleaseShader(shader);
}
} // namespace Rml
