#pragma once

namespace Rml
{

template<meta::OpenGL33ContextForward GL>
struct RendererOpenGL33<GL>::Impl
{
    ~Impl(void);

    Impl(Gl_Context_Provider get_gl);

    CompiledGeometryHandle
    CompileGeometry(Span<const Vertex> vertices, Span<const int> indices);

    void
    RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture);

    void
    ReleaseGeometry(CompiledGeometryHandle geometry);

    TextureHandle
    LoadTexture(Vector2i &texture_dimensions, const String &source);

    TextureHandle
    GenerateTexture(Span<const byte> source, Vector2i source_dimensions);

    void
    ReleaseTexture(TextureHandle texture);

    void
    EnableScissorRegion(bool enable);

    void
    SetScissorRegion(Rectanglei region);

    void
    EnableClipMask(bool enable);

    void
    RenderToClipMask(ClipMaskOperation operation, CompiledGeometryHandle geometry, Vector2f translation);

    void
    SetTransform(const Matrix4f *transform);

    LayerHandle
    PushLayer();

    void
    CompositeLayers(LayerHandle source, LayerHandle destination, BlendMode blend_mode, Span<const CompiledFilterHandle> filters);

    void
    PopLayer();

    TextureHandle
    SaveLayerAsTexture();

    CompiledFilterHandle
    SaveLayerAsMaskImage();

    CompiledFilterHandle
    CompileFilter(const String &name, const Dictionary &parameters);

    void
    ReleaseFilter(CompiledFilterHandle filter);

    CompiledShaderHandle
    CompileShader(const String &name, const Dictionary &parameters);

    void
    RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture);

    void
    ReleaseShader(CompiledShaderHandle shader);

  protected:

    Gl_Context_Provider get_gl_;
};

} // namespace Rml

#include <RmlUi/details/RenderOpenGL33/Impl.inl>
