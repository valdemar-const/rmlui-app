#include <skif/rmlui/app.hpp>
#include <skif/rmlui/config.hpp>

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Debugger.h>

#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/RenderInterfaceCompatibility.h>
#include <RmlUi/RendererGlad33.hpp>

#include <RmlUi/Core/Span.h>
#include <RmlUi/Core/DataModelHandle.h>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Log.h>

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <memory>

#include <cstdlib>

namespace skif::rmlui
{
App::App(int argc, char *argv[])
{
}

App::Return_Code
App::run(void)
{
    constexpr int width  = 640;
    constexpr int height = 480;

    App::Return_Code                     result             = EXIT_SUCCESS;
    GLFWwindow                          *main               = nullptr;
    std::unique_ptr<GladGLContext>       gl                 = nullptr;
    std::unique_ptr<Rml::RendererGlad33> render_impl        = nullptr;
    Rml::Context                        *context            = nullptr;
    Rml::ElementDocument                *document           = nullptr;
    bool                                 gl_loaded          = false;
    bool                                 font_loaded        = false;
    int                                  framebuffer_width  = width;
    int                                  framebuffer_height = height;

    struct Window_State
    {
        GladGLContext *gl      = nullptr;
        Rml::Context  *context = nullptr;
    };

    Window_State window_state = {};

    if (!glfwInit())
    {
        goto fail_init_glfw;
    }

    // glfwWindow + GladGLContext
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Core Profile
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Для macOS (если используется)

    main = glfwCreateWindow(width, height, "RmlUi App", nullptr, nullptr);
    if (!main)
    {
        goto fail_create_glfw_window;
    }

    glfwMakeContextCurrent(main);
    gl        = std::make_unique<GladGLContext>();
    gl_loaded = GLAD_MAKE_VERSION(3, 3) >= gladLoadGLContext(gl.get(), (GLADloadfunc)glfwGetProcAddress);
    if (!gl_loaded)
    {
        goto fail_init_glad_ctx;
    }

    glfwMakeContextCurrent(main);
    gl->ClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    gl->Enable(GL_BLEND);
    gl->BlendEquation(GL_FUNC_ADD);
    gl->BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glfwGetFramebufferSize(main, &framebuffer_width, &framebuffer_height);
    gl->Viewport(0, 0, framebuffer_width, framebuffer_height);

    // init RmlUi
    render_impl.reset(
            new Rml::RendererGlad33(
                    [w = main, gl = gl.get()](void) -> GladGLContext *
                    {
                        glfwMakeContextCurrent(w);
                        return gl;
                    }
            )
    );

    // init RmlUi
    Rml::SetRenderInterface(render_impl.get());
    Rml::Initialise();

    // create main RmlUi Context
    context = Rml::CreateContext("default", Rml::Vector2i(framebuffer_width, framebuffer_height));
    if (!context)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to create RmlUi context.");
        goto fail_init_rmlui;
    }

    // RmlUi load fonts
    font_loaded = Rml::LoadFontFace("assets/fonts/IBM_Plex_Mono/IBMPlexMono-Regular.ttf");
    if (!font_loaded)
    {
        Rml::Log::Message(Rml::Log::LT_WARNING, "Failed to load font face, fallback fonts may be used.");
    }

    // RmlUi Load Document
    document = context->LoadDocument("assets/ui/basic.rml");
    if (!document)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to load document: assets/ui/basic.rml");
        goto fail_load_document;
    }
    if (document)
    {
        document->Show();
    }

    window_state.gl      = gl.get();
    window_state.context = context;
    glfwSetWindowUserPointer(main, &window_state);

    glfwSetFramebufferSizeCallback(
            main,
            [](GLFWwindow *window, int new_width, int new_height)
            {
                auto *state = static_cast<Window_State *>(glfwGetWindowUserPointer(window));
                if (!state || !state->gl || !state->context)
                {
                    return;
                }

                state->gl->Viewport(0, 0, new_width, new_height);
                state->context->SetDimensions({new_width, new_height});
            }
    );

    glfwSetWindowRefreshCallback(
            main,
            [](GLFWwindow *window)
            {
                auto *state = static_cast<Window_State *>(glfwGetWindowUserPointer(window));
                if (!state || !state->gl || !state->context)
                {
                    return;
                }

                state->gl->Clear(GL_COLOR_BUFFER_BIT);
                state->context->Update();
                state->context->Render();
                glfwSwapBuffers(window);
            }
    );

    // prepare to main loop
    glfwShowWindow(main);
    while (!glfwWindowShouldClose(main))
    {
        glfwMakeContextCurrent(main);
        gl->Clear(GL_COLOR_BUFFER_BIT);

        // RmlUi process input

        // render rmlui gui
        context->Update();
        context->Render();

        glfwSwapBuffers(main);
        glfwPollEvents();
    }

    document->Close();
fail_load_document:
    Rml::RemoveContext(context->GetName());
fail_init_rmlui:
    Rml::Shutdown();
    glfwDestroyWindow(main);
    glfwTerminate();

    return result;

fail_init_glad_ctx:
    glfwDestroyWindow(main);
fail_create_glfw_window:
    glfwTerminate();
fail_init_glfw:
    return EXIT_FAILURE;
}

} // namespace skif::rmlui
