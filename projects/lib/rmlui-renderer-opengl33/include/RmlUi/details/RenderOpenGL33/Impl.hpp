#pragma once

#include <bitset>

namespace Rml
{

template<meta::OpenGL33ContextForward GL>
struct RendererOpenGL33<GL>::Impl
{
    using GLuint_t = unsigned int;

    enum class ProgramId
    {
        None,
        Color,
        Texture,
    };

    struct ProgramData
    {
        GLuint_t id = 0;
        int      uniform_transform = -1;
        int      uniform_translate = -1;
        int      uniform_tex       = -1;
    };

    struct CompiledGeometryData
    {
        GLuint_t vao        = 0;
        GLuint_t vbo        = 0;
        GLuint_t ibo        = 0;
        int      draw_count = 0;
    };

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

  private:

    bool InitializePrograms(void);
    void DestroyPrograms(void);

    bool CreateShader(GL &gl, GLuint_t &out_shader_id, unsigned int shader_type, const char *source);
    bool CreateProgram(GL &gl, ProgramData &out_program, const char *vertex_source, const char *fragment_source, bool textured);

    void UseProgram(ProgramId program_id);
    void SubmitTransformUniform(Vector2f translation);
    void UpdateViewportState(void);
    void SetScissor(Rectanglei region);

    static CompiledGeometryData *ToGeometry(CompiledGeometryHandle geometry);
    static CompiledGeometryHandle ToHandle(CompiledGeometryData *geometry);

    static TextureHandle ToTextureHandle(GLuint_t texture_id);
    static GLuint_t      ToTextureId(TextureHandle texture);

    static constexpr size_t kMaxPrograms = 8;

    ProgramData color_program_   = {};
    ProgramData texture_program_ = {};

    ProgramId active_program_ = ProgramId::None;

    std::bitset<kMaxPrograms> program_transform_dirty_ = {};

    Matrix4f transform_           = Matrix4f::Identity();
    Matrix4f projection_          = Matrix4f::Identity();
    Matrix4f local_transform_     = Matrix4f::Identity();
    bool     has_local_transform_ = false;

    Rectanglei scissor_state_ = Rectanglei::MakeInvalid();

    int viewport_x_      = 0;
    int viewport_y_      = 0;
    int viewport_width_  = 1;
    int viewport_height_ = 1;

    Gl_Context_Provider get_gl_;
};

} // namespace Rml

#include <RmlUi/details/RenderOpenGL33/Impl.inl>
