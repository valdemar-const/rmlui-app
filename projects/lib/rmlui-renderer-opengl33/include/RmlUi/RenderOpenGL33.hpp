#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <type_traits>

namespace Rml::meta
{
template<typename GL>
concept OpenGL33Context = requires(GL gl, float f) {
    { gl.ClearColor(f, f, f, f) } -> std::same_as<void>;
};

template<typename GL>
concept OpenGL33ContextForward = std::is_class_v<GL>;
} // namespace Rml::meta

namespace Rml
{

template<meta::OpenGL33ContextForward GL>
struct RendererOpenGL33 : public Rml::RenderInterface
{
    using Gl_Context_Provider = std::function<GL *(void)>;

  public:

    ~RendererOpenGL33() override;

    RendererOpenGL33(Gl_Context_Provider get_gl);

  public:

    CompiledGeometryHandle
    CompileGeometry(Span<const Vertex> vertices, Span<const int> indices) override;

    void
    RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture) override;

    void
    ReleaseGeometry(CompiledGeometryHandle geometry) override;

    TextureHandle
    LoadTexture(Vector2i &texture_dimensions, const String &source) override;

    TextureHandle
    GenerateTexture(Span<const byte> source, Vector2i source_dimensions) override;

    void
    ReleaseTexture(TextureHandle texture) override;

    void
    EnableScissorRegion(bool enable) override;

    void
    SetScissorRegion(Rectanglei region) override;

    void
    EnableClipMask(bool enable) override;

    void
    RenderToClipMask(ClipMaskOperation operation, CompiledGeometryHandle geometry, Vector2f translation) override;

    void
    SetTransform(const Matrix4f *transform) override;

    LayerHandle
    PushLayer() override;

    void
    CompositeLayers(LayerHandle source, LayerHandle destination, BlendMode blend_mode, Span<const CompiledFilterHandle> filters) override;

    void
    PopLayer() override;

    TextureHandle
    SaveLayerAsTexture() override;

    CompiledFilterHandle
    SaveLayerAsMaskImage() override;

    CompiledFilterHandle
    CompileFilter(const String &name, const Dictionary &parameters) override;

    void
    ReleaseFilter(CompiledFilterHandle filter) override;

    CompiledShaderHandle
    CompileShader(const String &name, const Dictionary &parameters) override;

    void
    RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture);

    void
    ReleaseShader(CompiledShaderHandle shader) override;

  protected:

    Gl_Context_Provider get_gl_;
};
} // namespace Rml

#ifdef RML_RENDERER_OPENGL33_IMPLEMENTATION
#include <RmlUi/details/RenderOpenGL33.inl>
#endif
