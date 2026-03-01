#include <skif/rmlui/app.hpp>

#include <skif/rmlui/config.hpp>

#include <implementation/window_manager_impl.hpp>
#include <implementation/window_impl.hpp>
#include <implementation/event_loop_impl.hpp>
#include <implementation/plugin_manager_impl.hpp>
#include <implementation/view_registry_impl.hpp>
#include <implementation/view_host_impl.hpp>
#include <implementation/input_manager_impl.hpp>
#include <implementation/editor_registry_impl.hpp>
#include <implementation/editor_host_impl.hpp>
#include <implementation/split_layout_impl.hpp>
#include <implementation/window_context.hpp>

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

#include <cassert>
#include <memory>
#include <cstdlib>

namespace skif::rmlui
{

struct App::Impl
{
    WindowConfig                      config;
    std::vector<std::string>          resource_directories;

    // Core
    std::unique_ptr<IWindowManager>   window_manager;
    std::unique_ptr<IEventLoop>       event_loop;
    std::unique_ptr<PluginManagerImpl> plugin_manager;
    std::unique_ptr<InputManagerImpl> input_manager;

    // Legacy View System (deprecated)
    std::unique_ptr<IViewRegistry>    view_registry;
    std::unique_ptr<IViewHost>        view_host;

    // Editor Platform
    std::unique_ptr<IEditorRegistry>  editor_registry;
    std::unique_ptr<IEditorHost>      editor_host;
    std::unique_ptr<ISplitLayout>     split_layout;

    // RmlUi state
    std::unique_ptr<GladGLContext>         gl;
    std::unique_ptr<Rml::RendererGlad33>  render_impl;
    Rml::Context* context = nullptr;

    // Единый контекст окна для GLFW callbacks
    WindowContext window_context;

    // Конфигурация
    std::string initial_view_name;
    std::string fallback_rml_path;
    std::unique_ptr<SplitNode> initial_layout;

    // ========================================================================
    // Private initialization methods
    // ========================================================================

    bool InitializeGL(IWindow* window)
    {
        window->MakeContextCurrent();
        gl = std::make_unique<GladGLContext>();

        const bool gl_loaded = GLAD_MAKE_VERSION(3, 3) >=
            gladLoadGLContext(gl.get(), (GLADloadfunc)glfwGetProcAddress);

        if (!gl_loaded)
        {
            return false;
        }

        gl->ClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        gl->Enable(GL_BLEND);
        gl->BlendEquation(GL_FUNC_ADD);
        gl->BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        const auto [fb_width, fb_height] = window->GetFramebufferSize();
        gl->Viewport(0, 0, fb_width, fb_height);

        return true;
    }

    bool InitializeRmlUi(IWindow* window)
    {
        const auto [fb_width, fb_height] = window->GetFramebufferSize();

        render_impl = std::make_unique<Rml::RendererGlad33>(
            [w = window, gl_ctx = gl.get()](void) -> GladGLContext*
            {
                w->MakeContextCurrent();
                return gl_ctx;
            }
        );

        Rml::SetRenderInterface(render_impl.get());
        Rml::Initialise();

        context = Rml::CreateContext("default", Rml::Vector2i(fb_width, fb_height));
        if (!context)
        {
            Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to create RmlUi context.");
            Rml::Shutdown();
            return false;
        }

        window_context.rml_context = context;

        // Legacy ViewHost
        view_host = std::make_unique<ViewHostImpl>(*view_registry);
        view_host->SetContext(context);

        // Editor Platform
        editor_host->SetContext(context);
        split_layout->SetContext(context);

        input_manager->SetContext(context);

        return true;
    }

    bool LoadFonts(const std::vector<std::string>& resource_dirs)
    {
        bool font_loaded = false;
        for (const auto& dir : resource_dirs)
        {
            const std::string font_path = dir + "/fonts/IBM_Plex_Mono/IBMPlexMono-Regular.ttf";
            font_loaded = Rml::LoadFontFace(font_path.c_str());
            if (font_loaded)
            {
                break;
            }
        }

        if (!font_loaded)
        {
            font_loaded = Rml::LoadFontFace("assets/fonts/IBM_Plex_Mono/IBMPlexMono-Regular.ttf");
        }

        if (!font_loaded)
        {
            Rml::Log::Message(Rml::Log::LT_WARNING, "Failed to load font face, fallback fonts may be used.");
        }

        return font_loaded;
    }

    void SetupGlfwCallbacks(IWindow* window)
    {
        auto* glfw_window = static_cast<GLFWwindow*>(window->GetNativeHandle());

        window_context.gl = gl.get();
        window_context.rml_context = context;
        window_context.input_manager = input_manager.get();

        glfwSetWindowUserPointer(glfw_window, &window_context);

        glfwSetFramebufferSizeCallback(
            glfw_window,
            [](GLFWwindow* gw, int new_width, int new_height)
            {
                auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(gw));
                if (!ctx || !ctx->gl || !ctx->rml_context)
                {
                    return;
                }

                ctx->gl->Viewport(0, 0, new_width, new_height);
                ctx->rml_context->SetDimensions({new_width, new_height});
            }
        );

        glfwSetWindowRefreshCallback(
            glfw_window,
            [](GLFWwindow* gw)
            {
                auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(gw));
                if (!ctx || !ctx->gl || !ctx->rml_context)
                {
                    return;
                }

                ctx->gl->Disable(GL_SCISSOR_TEST);
                ctx->gl->Clear(GL_COLOR_BUFFER_BIT);
                ctx->rml_context->Update();
                ctx->rml_context->Render();
                glfwSwapBuffers(gw);
            }
        );

        input_manager->SetWindow(glfw_window);
    }

    void SetupEventLoopCallbacks(IWindow* window)
    {
        event_loop->SetShouldCloseCheck(
            [window]() { return window->ShouldClose(); }
        );

        event_loop->OnUpdate(
            [this](float dt)
            {
                window_manager->PollEvents();
                input_manager->Update();

                // Рекурсивное обновление всех редакторов через SplitLayout
                split_layout->Update(dt);

                if (context)
                {
                    context->Update();
                }
            }
        );

        event_loop->OnRender(
            [this, window]()
            {
                window->MakeContextCurrent();

                // Сбрасываем scissor test перед очисткой — RmlUi может оставить его включённым
                gl->Disable(GL_SCISSOR_TEST);
                gl->Clear(GL_COLOR_BUFFER_BIT);

                if (context)
                {
                    context->Render();
                }

                window->SwapBuffers();
            }
        );

        event_loop->OnExit(
            [this]()
            {
                // 1. Уничтожаем редакторы
                split_layout->SetRoot(nullptr);
                editor_host->DestroyAll();

                // 2. Останавливаем плагины
                plugin_manager->StopPlugins();

                // 3. Очищаем RmlUi
                Rml::SetRenderInterface(nullptr);
                if (context)
                {
                    Rml::RemoveContext(context->GetName());
                }
                Rml::Shutdown();
                render_impl.reset();
            }
        );
    }

    void StartPluginsAndEditors()
    {
        // Запуск плагинов (они регистрируют редакторы через contribution point)
        plugin_manager->StartPlugins();

        // Инициализируем SplitLayout — создаём редакторы для всех leaf-узлов
        if (initial_layout)
        {
            split_layout->SetRoot(std::move(initial_layout));
            split_layout->Initialize();
        }
        else if (!initial_view_name.empty())
        {
            // Legacy: если задан initial_view, создаём простой layout с одним редактором
            // Проверяем, зарегистрирован ли как Editor
            if (editor_registry->GetDescriptor(initial_view_name))
            {
                split_layout->SetRoot(SplitNode::MakeLeaf(initial_view_name));
                split_layout->Initialize();
            }
            else
            {
                // Fallback к legacy ViewHost
                view_host->AttachView(initial_view_name, nullptr);
                view_host->ShowView(initial_view_name);
            }
        }

        // Fallback RML — загружаем только если нет ни layout, ни legacy view
        const bool has_layout = split_layout->GetRoot() != nullptr;
        const bool has_legacy_view = view_host && view_host->GetActiveView() != nullptr;

        if (!has_layout && !has_legacy_view && !fallback_rml_path.empty())
        {
            Rml::Log::Message(Rml::Log::LT_INFO, "Trying to load fallback RML: %s", fallback_rml_path.c_str());
            auto* document = context->LoadDocument(fallback_rml_path.c_str());
            if (document)
            {
                document->Show();
            }
            else
            {
                Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to load fallback RML: %s", fallback_rml_path.c_str());
            }
        }
    }

    void Cleanup(std::shared_ptr<IWindow>& window)
    {
        plugin_manager->Shutdown();
        window_manager->DestroyWindow(window);
        window_manager->Shutdown();
    }
};

// ============================================================================
// App public methods
// ============================================================================

App::App(int argc, char* argv[])
    : pimpl_(std::make_unique<Impl>())
{
    // Core
    pimpl_->window_manager = std::make_unique<WindowManagerImpl>();
    pimpl_->event_loop     = std::make_unique<EventLoopImpl>();
    pimpl_->plugin_manager = std::make_unique<PluginManagerImpl>();
    pimpl_->input_manager  = std::make_unique<InputManagerImpl>();

    // Legacy View System
    pimpl_->view_registry  = std::make_unique<ViewRegistryImpl>();

    // Editor Platform
    pimpl_->editor_registry = std::make_unique<EditorRegistryImpl>();
    pimpl_->editor_host     = std::make_unique<EditorHostImpl>(
        *pimpl_->editor_registry
    );
    pimpl_->split_layout    = std::make_unique<SplitLayoutImpl>();
    pimpl_->split_layout->SetEditorHost(pimpl_->editor_host.get());

    // Подключаем registries к PluginManager
    pimpl_->plugin_manager->SetViewRegistry(pimpl_->view_registry.get());
    pimpl_->plugin_manager->SetEditorRegistry(pimpl_->editor_registry.get());
}

App::~App() = default;

IWindowManager&
App::GetWindowManager() noexcept
{
    return *pimpl_->window_manager;
}

IPluginManager&
App::GetPluginManager() noexcept
{
    return *pimpl_->plugin_manager;
}

IInputManager&
App::GetInputManager() noexcept
{
    return *pimpl_->input_manager;
}

const WindowConfig&
App::GetConfig() const noexcept
{
    return pimpl_->config;
}

WindowConfig&
App::GetConfig() noexcept
{
    return pimpl_->config;
}

// Editor Platform
IEditorRegistry&
App::GetEditorRegistry() noexcept
{
    return *pimpl_->editor_registry;
}

IEditorHost&
App::GetEditorHost() noexcept
{
    return *pimpl_->editor_host;
}

ISplitLayout&
App::GetSplitLayout() noexcept
{
    return *pimpl_->split_layout;
}

void
App::SetInitialLayout(std::unique_ptr<SplitNode> root)
{
    pimpl_->initial_layout = std::move(root);
}

// Legacy View System
IViewRegistry&
App::GetViewRegistry() noexcept
{
    return *pimpl_->view_registry;
}

IViewHost&
App::GetViewHost() noexcept
{
    assert(pimpl_->view_host && "GetViewHost() called before run()");
    return *pimpl_->view_host;
}

// Resources
void
App::AddResourceDirectory(std::string_view path)
{
    pimpl_->resource_directories.emplace_back(path);
}

const std::vector<std::string>&
App::GetResourceDirectories() const noexcept
{
    return pimpl_->resource_directories;
}

void
App::SetInitialView(std::string_view view_name)
{
    pimpl_->initial_view_name = view_name;
}

void
App::SetFallbackRml(std::string_view rml_path)
{
    pimpl_->fallback_rml_path = rml_path;
}

// ============================================================================
// Main run method
// ============================================================================

App::Return_Code
App::run()
{
    if (!pimpl_->window_manager->Initialize())
    {
        return EXIT_FAILURE;
    }

    if (!pimpl_->plugin_manager->Initialize())
    {
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    auto window = pimpl_->window_manager->CreateWindow(pimpl_->config);
    if (!window)
    {
        pimpl_->plugin_manager->Shutdown();
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    if (!pimpl_->InitializeGL(window.get()))
    {
        pimpl_->window_manager->DestroyWindow(window);
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    pimpl_->SetupGlfwCallbacks(window.get());

    if (!pimpl_->InitializeRmlUi(window.get()))
    {
        pimpl_->window_manager->DestroyWindow(window);
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    pimpl_->LoadFonts(pimpl_->resource_directories);
    pimpl_->SetupEventLoopCallbacks(window.get());
    pimpl_->StartPluginsAndEditors();

    pimpl_->event_loop->Run();

    pimpl_->Cleanup(window);

    return EXIT_SUCCESS;
}

} // namespace skif::rmlui
