#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <functional>
#include <memory>
#include <type_traits>

namespace Rml::meta
{
template<typename GL>
concept OpenGL33Context =
        requires(
                GL           gl,
                float        f,
                int          i,
                unsigned int ui,
                const char  *source,
                const void  *ptr,
                void        *mut_ptr
        ) {
            { gl.ClearColor(f, f, f, f) } -> std::same_as<void>;

            { gl.CreateShader(ui) } -> std::convertible_to<unsigned int>;
            { gl.ShaderSource(ui, i, &source, nullptr) } -> std::same_as<void>;
            { gl.CompileShader(ui) } -> std::same_as<void>;
            { gl.GetShaderiv(ui, ui, &i) } -> std::same_as<void>;
            { gl.GetShaderInfoLog(ui, i, nullptr, static_cast<char *>(mut_ptr)) } -> std::same_as<void>;
            { gl.DeleteShader(ui) } -> std::same_as<void>;

            { gl.CreateProgram() } -> std::convertible_to<unsigned int>;
            { gl.BindAttribLocation(ui, ui, source) } -> std::same_as<void>;
            { gl.AttachShader(ui, ui) } -> std::same_as<void>;
            { gl.LinkProgram(ui) } -> std::same_as<void>;
            { gl.DetachShader(ui, ui) } -> std::same_as<void>;
            { gl.GetProgramiv(ui, ui, &i) } -> std::same_as<void>;
            { gl.GetProgramInfoLog(ui, i, nullptr, static_cast<char *>(mut_ptr)) } -> std::same_as<void>;
            { gl.DeleteProgram(ui) } -> std::same_as<void>;
            { gl.GetUniformLocation(ui, source) } -> std::convertible_to<int>;
            { gl.UseProgram(ui) } -> std::same_as<void>;
            { gl.Uniform1i(i, i) } -> std::same_as<void>;
            { gl.Uniform2fv(i, i, static_cast<const float *>(ptr)) } -> std::same_as<void>;
            { gl.UniformMatrix4fv(i, i, static_cast<unsigned char>(0), static_cast<const float *>(ptr)) } -> std::same_as<void>;

            { gl.GenVertexArrays(i, static_cast<unsigned int *>(mut_ptr)) } -> std::same_as<void>;
            { gl.GenBuffers(i, static_cast<unsigned int *>(mut_ptr)) } -> std::same_as<void>;
            { gl.BindVertexArray(ui) } -> std::same_as<void>;
            { gl.BindBuffer(ui, ui) } -> std::same_as<void>;
            { gl.BufferData(ui, static_cast<std::ptrdiff_t>(0), ptr, ui) } -> std::same_as<void>;
            { gl.EnableVertexAttribArray(ui) } -> std::same_as<void>;
            { gl.VertexAttribPointer(ui, i, ui, static_cast<unsigned char>(0), i, ptr) } -> std::same_as<void>;
            { gl.DrawElements(ui, i, ui, ptr) } -> std::same_as<void>;
            { gl.DeleteVertexArrays(i, static_cast<const unsigned int *>(ptr)) } -> std::same_as<void>;
            { gl.DeleteBuffers(i, static_cast<const unsigned int *>(ptr)) } -> std::same_as<void>;

            { gl.GenTextures(i, static_cast<unsigned int *>(mut_ptr)) } -> std::same_as<void>;
            { gl.BindTexture(ui, ui) } -> std::same_as<void>;
            { gl.TexImage2D(ui, i, i, i, i, i, ui, ui, ptr) } -> std::same_as<void>;
            { gl.TexParameteri(ui, ui, i) } -> std::same_as<void>;
            { gl.DeleteTextures(i, static_cast<const unsigned int *>(ptr)) } -> std::same_as<void>;

            { gl.Enable(ui) } -> std::same_as<void>;
            { gl.Disable(ui) } -> std::same_as<void>;
            { gl.Scissor(i, i, i, i) } -> std::same_as<void>;
            { gl.GetIntegerv(ui, &i) } -> std::same_as<void>;
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
    RenderShader(CompiledShaderHandle shader, CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture) override;

    void
    ReleaseShader(CompiledShaderHandle shader) override;

  private:

    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
} // namespace Rml

#ifdef RML_RENDERER_OPENGL33_IMPLEMENTATION
#include <RmlUi/details/RenderOpenGL33.inl>
#endif
