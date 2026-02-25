#pragma once

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Math.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace Rml
{

namespace
{
// GL enum constants wrapped as dependent variable templates for ODR-isolation in .inl.
// NOTE: The TU that triggers instantiation MUST include the appropriate GL header
// (e.g. <glad/gl.h>) before this file is transitively included.
template<typename GL>
constexpr unsigned int kGlVertexShader = GL_VERTEX_SHADER;
template<typename GL>
constexpr unsigned int kGlFragmentShader = GL_FRAGMENT_SHADER;
template<typename GL>
constexpr unsigned int kGlCompileStatus = GL_COMPILE_STATUS;
template<typename GL>
constexpr unsigned int kGlInfoLogLength = GL_INFO_LOG_LENGTH;
template<typename GL>
constexpr unsigned int kGlLinkStatus = GL_LINK_STATUS;

template<typename GL>
constexpr unsigned int kGlArrayBuffer = GL_ARRAY_BUFFER;
template<typename GL>
constexpr unsigned int kGlElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER;
template<typename GL>
constexpr unsigned int kGlStaticDraw = GL_STATIC_DRAW;

template<typename GL>
constexpr unsigned int kGlTriangles = GL_TRIANGLES;
template<typename GL>
constexpr unsigned int kGlUnsignedInt = GL_UNSIGNED_INT;
template<typename GL>
constexpr unsigned int kGlFloat = GL_FLOAT;
template<typename GL>
constexpr unsigned int kGlUnsignedByte = GL_UNSIGNED_BYTE;
template<typename GL>
constexpr unsigned int kGlFalse = GL_FALSE;
template<typename GL>
constexpr unsigned int kGlTrue = GL_TRUE;

template<typename GL>
constexpr unsigned int kGlTexture2D = GL_TEXTURE_2D;
template<typename GL>
constexpr unsigned int kGlRgba8 = GL_RGBA8;
template<typename GL>
constexpr unsigned int kGlRgba = GL_RGBA;
template<typename GL>
constexpr unsigned int kGlLinear = GL_LINEAR;
template<typename GL>
constexpr unsigned int kGlRepeat = GL_REPEAT;
template<typename GL>
constexpr unsigned int kGlTextureMinFilter = GL_TEXTURE_MIN_FILTER;
template<typename GL>
constexpr unsigned int kGlTextureMagFilter = GL_TEXTURE_MAG_FILTER;
template<typename GL>
constexpr unsigned int kGlTextureWrapS = GL_TEXTURE_WRAP_S;
template<typename GL>
constexpr unsigned int kGlTextureWrapT = GL_TEXTURE_WRAP_T;

template<typename GL>
constexpr unsigned int kGlScissorTest = GL_SCISSOR_TEST;
template<typename GL>
constexpr unsigned int kGlViewport = GL_VIEWPORT;

inline constexpr const char *kVertexShaderMain = R"(
#version 330
uniform vec2 _translate;
uniform mat4 _transform;

in vec2 inPosition;
in vec4 inColor0;
in vec2 inTexCoord0;

out vec2 fragTexCoord;
out vec4 fragColor;

void main() {
    fragTexCoord = inTexCoord0;
    fragColor = inColor0;

    vec2 translatedPos = inPosition + _translate;
    vec4 outPos = _transform * vec4(translatedPos, 0.0, 1.0);

    gl_Position = outPos;
}
)";

inline constexpr const char *kFragmentShaderColor = R"(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    finalColor = fragColor;
}
)";

inline constexpr const char *kFragmentShaderTexture = R"(
#version 330
uniform sampler2D _tex;
in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;

void main() {
    vec4 texColor = texture(_tex, fragTexCoord);
    finalColor = fragColor * texColor;
}
)";

// TGA header — binary layout, must match on-disk format exactly.
// #pragma pack is supported by GCC, Clang, and MSVC.
#pragma pack(push, 1)
struct TgaHeader
{
    char      id_length;
    char      color_map_type;
    char      data_type;
    short int color_map_origin;
    short int color_map_length;
    char      color_map_depth;
    short int x_origin;
    short int y_origin;
    short int width;
    short int height;
    char      bits_per_pixel;
    char      image_descriptor;
};
#pragma pack(pop)
static_assert(sizeof(TgaHeader) == 18, "TgaHeader must be exactly 18 bytes to match TGA on-disk format.");

} // namespace

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::Impl::~Impl()
{
    DestroyPrograms();
}

template<meta::OpenGL33ContextForward GL>
inline RendererOpenGL33<GL>::Impl::Impl(Gl_Context_Provider get_gl)
    : get_gl_(std::move(get_gl))
{
    if (!get_gl_)
    {
        throw std::invalid_argument("Non nullptr gl context provider should be set");
    }

    if (!InitializePrograms())
    {
        throw std::runtime_error("Cannot initialize OpenGL programs for RendererOpenGL33");
    }

    // Initialize projection from current viewport and set identity transform.
    UpdateViewportState();
    SetTransform(nullptr);
}

template<meta::OpenGL33ContextForward GL>
inline CompiledGeometryHandle
RendererOpenGL33<GL>::Impl::CompileGeometry(Span<const Vertex> vertices, Span<const int> indices)
{
    auto &gl = *get_gl_();

    auto *geometry = new CompiledGeometryData();

    gl.GenVertexArrays(1, &geometry->vao);
    gl.GenBuffers(1, &geometry->vbo);
    gl.GenBuffers(1, &geometry->ibo);

    gl.BindVertexArray(geometry->vao);

    gl.BindBuffer(kGlArrayBuffer<GL>, geometry->vbo);
    gl.BufferData(kGlArrayBuffer<GL>, static_cast<std::ptrdiff_t>(sizeof(Vertex) * vertices.size()), static_cast<const void *>(vertices.data()), kGlStaticDraw<GL>);

    constexpr unsigned int kAttrPosition = 0;
    constexpr unsigned int kAttrColor0   = 1;
    constexpr unsigned int kAttrTexCoord = 2;

    gl.EnableVertexAttribArray(kAttrPosition);
    gl.VertexAttribPointer(kAttrPosition,
                           2,
                           kGlFloat<GL>,
                           static_cast<unsigned char>(kGlFalse<GL>),
                           static_cast<int>(sizeof(Vertex)),
                           reinterpret_cast<const void *>(offsetof(Vertex, position)));

    gl.EnableVertexAttribArray(kAttrColor0);
    gl.VertexAttribPointer(kAttrColor0,
                           4,
                           kGlUnsignedByte<GL>,
                           static_cast<unsigned char>(kGlTrue<GL>),
                           static_cast<int>(sizeof(Vertex)),
                           reinterpret_cast<const void *>(offsetof(Vertex, colour)));

    gl.EnableVertexAttribArray(kAttrTexCoord);
    gl.VertexAttribPointer(kAttrTexCoord,
                           2,
                           kGlFloat<GL>,
                           static_cast<unsigned char>(kGlFalse<GL>),
                           static_cast<int>(sizeof(Vertex)),
                           reinterpret_cast<const void *>(offsetof(Vertex, tex_coord)));

    gl.BindBuffer(kGlElementArrayBuffer<GL>, geometry->ibo);
    gl.BufferData(kGlElementArrayBuffer<GL>, static_cast<std::ptrdiff_t>(sizeof(int) * indices.size()), static_cast<const void *>(indices.data()), kGlStaticDraw<GL>);

    gl.BindVertexArray(0);
    gl.BindBuffer(kGlArrayBuffer<GL>, 0);

    geometry->draw_count = static_cast<int>(indices.size());

    return ToHandle(geometry);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::RenderGeometry(CompiledGeometryHandle geometry, Vector2f translation, TextureHandle texture)
{
    auto &gl = *get_gl_();

    if (!geometry)
    {
        return;
    }

    // Lazily sync projection with the current GL viewport (handles window resize, DPI changes).
    // The actual recalculation only happens when the viewport dimensions change.
    UpdateViewportState();

    auto *compiled_geometry = ToGeometry(geometry);

    if (texture)
    {
        UseProgram(ProgramId::Texture);
        gl.BindTexture(kGlTexture2D<GL>, ToTextureId(texture));
    }
    else
    {
        UseProgram(ProgramId::Color);
        gl.BindTexture(kGlTexture2D<GL>, 0);
    }

    SubmitTransformUniform(translation);

    gl.BindVertexArray(compiled_geometry->vao);
    gl.DrawElements(kGlTriangles<GL>, compiled_geometry->draw_count, kGlUnsignedInt<GL>, nullptr);
    gl.BindVertexArray(0);

    // Unbind texture to leave clean GL state for subsequent calls.
    gl.BindTexture(kGlTexture2D<GL>, 0);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseGeometry(CompiledGeometryHandle geometry)
{
    auto &gl = *get_gl_();

    if (!geometry)
    {
        return;
    }

    auto *compiled_geometry = ToGeometry(geometry);

    gl.DeleteVertexArrays(1, &compiled_geometry->vao);
    gl.DeleteBuffers(1, &compiled_geometry->vbo);
    gl.DeleteBuffers(1, &compiled_geometry->ibo);

    delete compiled_geometry;
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::LoadTexture(Vector2i &texture_dimensions, const String &source)
{
    auto *file_interface = Rml::GetFileInterface();
    if (!file_interface)
    {
        Log::Message(Log::LT_ERROR, "RmlUi file interface is not available, cannot load texture '%s'.", source.c_str());
        return {};
    }

    FileHandle file = file_interface->Open(source);
    if (!file)
    {
        return {};
    }

    file_interface->Seek(file, 0, SEEK_END);
    const size_t file_size = file_interface->Tell(file);
    file_interface->Seek(file, 0, SEEK_SET);

    if (file_size <= sizeof(TgaHeader))
    {
        Log::Message(Log::LT_ERROR, "Texture file '%s' is too small for TGA header.", source.c_str());
        file_interface->Close(file);
        return {};
    }

    std::vector<byte> buffer(file_size);
    file_interface->Read(buffer.data(), file_size, file);
    file_interface->Close(file);

    TgaHeader header = {};
    std::memcpy(&header, buffer.data(), sizeof(TgaHeader));

    if (header.data_type != 2)
    {
        Log::Message(Log::LT_ERROR, "Texture '%s': only uncompressed TGA is supported.", source.c_str());
        return {};
    }

    const int bytes_per_pixel = header.bits_per_pixel / 8;
    if (bytes_per_pixel != 3 && bytes_per_pixel != 4)
    {
        Log::Message(Log::LT_ERROR, "Texture '%s': only 24/32 bpp TGA is supported.", source.c_str());
        return {};
    }

    const int width  = header.width;
    const int height = header.height;
    if (width <= 0 || height <= 0)
    {
        Log::Message(Log::LT_ERROR, "Texture '%s': invalid TGA dimensions.", source.c_str());
        return {};
    }

    // Validate that the file contains enough pixel data.
    const size_t expected_pixel_data_size = static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(bytes_per_pixel);
    const size_t available_pixel_data     = file_size - sizeof(TgaHeader);
    if (available_pixel_data < expected_pixel_data_size)
    {
        Log::Message(Log::LT_ERROR, "Texture '%s': file truncated, expected %zu bytes of pixel data but only %zu available.",
                     source.c_str(), expected_pixel_data_size, available_pixel_data);
        return {};
    }

    const size_t pixels_rgba_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
    std::vector<byte> rgba_pixels(pixels_rgba_size);

    const byte *src = buffer.data() + sizeof(TgaHeader);
    const bool  top_to_bottom = (header.image_descriptor & 32) != 0;

    for (int y = 0; y < height; ++y)
    {
        int src_index = y * width * bytes_per_pixel;
        int dst_index = (top_to_bottom ? y : (height - y - 1)) * width * 4;

        for (int x = 0; x < width; ++x)
        {
            byte r = src[src_index + 2];
            byte g = src[src_index + 1];
            byte b = src[src_index + 0];
            byte a = (bytes_per_pixel == 4 ? src[src_index + 3] : byte(255));

            // Premultiply alpha.
            r = static_cast<byte>((static_cast<unsigned int>(r) * static_cast<unsigned int>(a)) / 255u);
            g = static_cast<byte>((static_cast<unsigned int>(g) * static_cast<unsigned int>(a)) / 255u);
            b = static_cast<byte>((static_cast<unsigned int>(b) * static_cast<unsigned int>(a)) / 255u);

            rgba_pixels[dst_index + 0] = r;
            rgba_pixels[dst_index + 1] = g;
            rgba_pixels[dst_index + 2] = b;
            rgba_pixels[dst_index + 3] = a;

            src_index += bytes_per_pixel;
            dst_index += 4;
        }
    }

    texture_dimensions = {width, height};
    return GenerateTexture(Span<const byte>(rgba_pixels.data(), rgba_pixels.size()), texture_dimensions);
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::GenerateTexture(Span<const byte> source, Vector2i source_dimensions)
{
    auto &gl = *get_gl_();

    if (!source.data() || source_dimensions.x <= 0 || source_dimensions.y <= 0)
    {
        return {};
    }

    GLuint_t texture_id = 0;
    gl.GenTextures(1, &texture_id);
    if (!texture_id)
    {
        return {};
    }

    gl.BindTexture(kGlTexture2D<GL>, texture_id);
    gl.TexImage2D(kGlTexture2D<GL>,
                  0,
                  static_cast<int>(kGlRgba8<GL>),
                  source_dimensions.x,
                  source_dimensions.y,
                  0,
                  kGlRgba<GL>,
                  kGlUnsignedByte<GL>,
                  static_cast<const void *>(source.data()));
    gl.TexParameteri(kGlTexture2D<GL>, kGlTextureMinFilter<GL>, static_cast<int>(kGlLinear<GL>));
    gl.TexParameteri(kGlTexture2D<GL>, kGlTextureMagFilter<GL>, static_cast<int>(kGlLinear<GL>));
    gl.TexParameteri(kGlTexture2D<GL>, kGlTextureWrapS<GL>, static_cast<int>(kGlRepeat<GL>));
    gl.TexParameteri(kGlTexture2D<GL>, kGlTextureWrapT<GL>, static_cast<int>(kGlRepeat<GL>));
    gl.BindTexture(kGlTexture2D<GL>, 0);

    return ToTextureHandle(texture_id);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseTexture(TextureHandle texture)
{
    auto &gl = *get_gl_();

    if (!texture)
    {
        return;
    }

    const GLuint_t texture_id = ToTextureId(texture);
    gl.DeleteTextures(1, &texture_id);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::EnableScissorRegion(bool enable)
{
    // Following the reference implementation pattern:
    // enable=true is always immediately followed by SetScissorRegion(), so we only act on disable.
    if (!enable)
    {
        SetScissor(Rectanglei::MakeInvalid());
    }
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::SetScissorRegion(Rectanglei region)
{
    SetScissor(region);
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::EnableClipMask([[maybe_unused]] bool enable)
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: EnableClipMask is not implemented.");
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::RenderToClipMask(
        [[maybe_unused]] ClipMaskOperation operation,
        [[maybe_unused]] CompiledGeometryHandle geometry,
        [[maybe_unused]] Vector2f translation)
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: RenderToClipMask is not implemented.");
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::SetTransform(const Matrix4f *transform)
{
    has_local_transform_ = (transform != nullptr);
    local_transform_     = (transform ? *transform : Matrix4f::Identity());

    transform_ = projection_ * local_transform_;
    program_transform_dirty_.set();
}

template<meta::OpenGL33ContextForward GL>
inline LayerHandle
RendererOpenGL33<GL>::Impl::PushLayer()
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: PushLayer is not implemented.");
    return {};
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::CompositeLayers(
        [[maybe_unused]] LayerHandle source,
        [[maybe_unused]] LayerHandle destination,
        [[maybe_unused]] BlendMode blend_mode,
        [[maybe_unused]] Span<const CompiledFilterHandle> filters)
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: CompositeLayers is not implemented.");
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::PopLayer()
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: PopLayer is not implemented.");
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::SaveLayerAsTexture()
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: SaveLayerAsTexture is not implemented.");
    return {};
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::Impl::SaveLayerAsMaskImage()
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: SaveLayerAsMaskImage is not implemented.");
    return {};
}

template<meta::OpenGL33ContextForward GL>
inline CompiledFilterHandle
RendererOpenGL33<GL>::Impl::CompileFilter(
        [[maybe_unused]] const String &name,
        [[maybe_unused]] const Dictionary &parameters)
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: CompileFilter is not implemented.");
    return {};
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseFilter([[maybe_unused]] CompiledFilterHandle filter)
{
}

template<meta::OpenGL33ContextForward GL>
inline CompiledShaderHandle
RendererOpenGL33<GL>::Impl::CompileShader(
        [[maybe_unused]] const String &name,
        [[maybe_unused]] const Dictionary &parameters)
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: CompileShader is not implemented.");
    return {};
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::RenderShader(
        [[maybe_unused]] CompiledShaderHandle shader,
        [[maybe_unused]] CompiledGeometryHandle geometry,
        [[maybe_unused]] Vector2f translation,
        [[maybe_unused]] TextureHandle texture)
{
    Log::Message(Log::LT_WARNING, "RendererOpenGL33 MVP: RenderShader is not implemented.");
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::ReleaseShader([[maybe_unused]] CompiledShaderHandle shader)
{
}

template<meta::OpenGL33ContextForward GL>
inline bool
RendererOpenGL33<GL>::Impl::InitializePrograms(void)
{
    auto &gl = *get_gl_();

    if (!CreateProgram(gl, color_program_, kVertexShaderMain, kFragmentShaderColor, false))
    {
        return false;
    }

    if (!CreateProgram(gl, texture_program_, kVertexShaderMain, kFragmentShaderTexture, true))
    {
        return false;
    }

    active_program_ = ProgramId::None;
    program_transform_dirty_.set();

    return true;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::DestroyPrograms(void)
{
    auto &gl = *get_gl_();

    if (color_program_.id)
    {
        gl.DeleteProgram(color_program_.id);
        color_program_ = {};
    }

    if (texture_program_.id)
    {
        gl.DeleteProgram(texture_program_.id);
        texture_program_ = {};
    }

    active_program_ = ProgramId::None;
}

template<meta::OpenGL33ContextForward GL>
inline bool
RendererOpenGL33<GL>::Impl::CreateShader(GL &gl, GLuint_t &out_shader_id, unsigned int shader_type, const char *source)
{
    const GLuint_t shader = gl.CreateShader(shader_type);
    gl.ShaderSource(shader, 1, &source, nullptr);
    gl.CompileShader(shader);

    int status = 0;
    gl.GetShaderiv(shader, kGlCompileStatus<GL>, &status);
    if (!status)
    {
        int log_length = 0;
        gl.GetShaderiv(shader, kGlInfoLogLength<GL>, &log_length);
        std::vector<char> log(log_length + 1, '\0');
        gl.GetShaderInfoLog(shader, log_length, nullptr, log.data());
        Log::Message(Log::LT_ERROR, "RendererOpenGL33 shader compile failed: %s", log.data());
        gl.DeleteShader(shader);
        return false;
    }

    out_shader_id = shader;
    return true;
}

template<meta::OpenGL33ContextForward GL>
inline bool
RendererOpenGL33<GL>::Impl::CreateProgram(GL &gl, ProgramData &out_program, const char *vertex_source, const char *fragment_source, bool textured)
{
    GLuint_t vertex_shader   = 0;
    GLuint_t fragment_shader = 0;

    if (!CreateShader(gl, vertex_shader, kGlVertexShader<GL>, vertex_source))
    {
        return false;
    }

    if (!CreateShader(gl, fragment_shader, kGlFragmentShader<GL>, fragment_source))
    {
        gl.DeleteShader(vertex_shader);
        return false;
    }

    const GLuint_t program = gl.CreateProgram();

    gl.BindAttribLocation(program, 0, "inPosition");
    gl.BindAttribLocation(program, 1, "inColor0");
    gl.BindAttribLocation(program, 2, "inTexCoord0");

    gl.AttachShader(program, vertex_shader);
    gl.AttachShader(program, fragment_shader);
    gl.LinkProgram(program);
    gl.DetachShader(program, vertex_shader);
    gl.DetachShader(program, fragment_shader);

    gl.DeleteShader(vertex_shader);
    gl.DeleteShader(fragment_shader);

    int status = 0;
    gl.GetProgramiv(program, kGlLinkStatus<GL>, &status);
    if (!status)
    {
        int log_length = 0;
        gl.GetProgramiv(program, kGlInfoLogLength<GL>, &log_length);
        std::vector<char> log(log_length + 1, '\0');
        gl.GetProgramInfoLog(program, log_length, nullptr, log.data());
        Log::Message(Log::LT_ERROR, "RendererOpenGL33 program link failed: %s", log.data());
        gl.DeleteProgram(program);
        return false;
    }

    out_program.id                = program;
    out_program.uniform_transform = gl.GetUniformLocation(program, "_transform");
    out_program.uniform_translate = gl.GetUniformLocation(program, "_translate");
    out_program.uniform_tex       = gl.GetUniformLocation(program, "_tex");

    if (textured && out_program.uniform_tex >= 0)
    {
        gl.UseProgram(program);
        gl.Uniform1i(out_program.uniform_tex, 0);
    }

    gl.UseProgram(0);

    return true;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::UseProgram(ProgramId program_id)
{
    auto &gl = *get_gl_();

    if (active_program_ == program_id)
    {
        return;
    }

    if (program_id == ProgramId::None)
    {
        gl.UseProgram(0);
        active_program_ = program_id;
        return;
    }

    ProgramData *program = nullptr;
    switch (program_id)
    {
        case ProgramId::Color:
            program = &color_program_;
            break;
        case ProgramId::Texture:
            program = &texture_program_;
            break;
        case ProgramId::None:
            break;
    }

    if (!program || !program->id)
    {
        return;
    }

    gl.UseProgram(program->id);
    active_program_ = program_id;
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::SubmitTransformUniform(Vector2f translation)
{
    auto &gl = *get_gl_();

    ProgramData *program = nullptr;
    switch (active_program_)
    {
        case ProgramId::Color:
            program = &color_program_;
            break;
        case ProgramId::Texture:
            program = &texture_program_;
            break;
        case ProgramId::None:
            return;
    }

    const size_t program_index = static_cast<size_t>(active_program_);
    if (program_index < kMaxPrograms && program_transform_dirty_.test(program_index) && program->uniform_transform >= 0)
    {
        gl.UniformMatrix4fv(program->uniform_transform, 1, static_cast<unsigned char>(kGlFalse<GL>), transform_.data());
        program_transform_dirty_.set(program_index, false);
    }

    if (program->uniform_translate >= 0)
    {
        gl.Uniform2fv(program->uniform_translate, 1, &translation.x);
    }
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::UpdateViewportState(void)
{
    auto &gl = *get_gl_();

    std::array<int, 4> viewport = {0, 0, 1, 1};
    gl.GetIntegerv(kGlViewport<GL>, viewport.data());

    const int new_width  = Math::Max(viewport[2], 1);
    const int new_height = Math::Max(viewport[3], 1);

    // Only recalculate projection when viewport actually changed.
    if (viewport[0] == viewport_x_ && viewport[1] == viewport_y_ &&
        new_width == viewport_width_ && new_height == viewport_height_)
    {
        return;
    }

    viewport_x_      = viewport[0];
    viewport_y_      = viewport[1];
    viewport_width_  = new_width;
    viewport_height_ = new_height;

    projection_ = Matrix4f::ProjectOrtho(0.f, static_cast<float>(viewport_width_), static_cast<float>(viewport_height_), 0.f, -10000.f, 10000.f);
    transform_  = projection_ * (has_local_transform_ ? local_transform_ : Matrix4f::Identity());
    program_transform_dirty_.set();
}

template<meta::OpenGL33ContextForward GL>
inline void
RendererOpenGL33<GL>::Impl::SetScissor(Rectanglei region)
{
    auto &gl = *get_gl_();

    // Toggle GL_SCISSOR_TEST only when validity changes.
    if (region.Valid() != scissor_state_.Valid())
    {
        if (region.Valid())
            gl.Enable(kGlScissorTest<GL>);
        else
            gl.Disable(kGlScissorTest<GL>);
    }

    if (region.Valid() && region != scissor_state_)
    {
        // Clamp to viewport to avoid driver issues (WebGL in particular).
        const int x = Math::Clamp(region.Left(), 0, viewport_width_);
        const int y = Math::Clamp(viewport_height_ - region.Bottom(), 0, viewport_height_);
        const int w = Math::Clamp(region.Width(), 0, viewport_width_);
        const int h = Math::Clamp(region.Height(), 0, viewport_height_);

        gl.Scissor(x, y, w, h);
    }

    scissor_state_ = region;
}

template<meta::OpenGL33ContextForward GL>
inline typename RendererOpenGL33<GL>::Impl::CompiledGeometryData *
RendererOpenGL33<GL>::Impl::ToGeometry(CompiledGeometryHandle geometry)
{
    return reinterpret_cast<CompiledGeometryData *>(geometry);
}

template<meta::OpenGL33ContextForward GL>
inline CompiledGeometryHandle
RendererOpenGL33<GL>::Impl::ToHandle(CompiledGeometryData *geometry)
{
    return reinterpret_cast<CompiledGeometryHandle>(geometry);
}

template<meta::OpenGL33ContextForward GL>
inline TextureHandle
RendererOpenGL33<GL>::Impl::ToTextureHandle(GLuint_t texture_id)
{
    return static_cast<TextureHandle>(texture_id);
}

template<meta::OpenGL33ContextForward GL>
inline typename RendererOpenGL33<GL>::Impl::GLuint_t
RendererOpenGL33<GL>::Impl::ToTextureId(TextureHandle texture)
{
    return static_cast<GLuint_t>(texture);
}

} // namespace Rml
