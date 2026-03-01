#include <skif/rmlui/app.hpp>

#include <skif/rmlui/config.hpp>

#include <implementation/window_manager_impl.hpp>
#include <implementation/window_impl.hpp>
#include <implementation/event_loop_impl.hpp>
#include <implementation/plugin_manager_impl.hpp>
#include <implementation/view_registry_impl.hpp>
#include <implementation/view_host_impl.hpp>
#include <implementation/input_manager_impl.hpp>
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

#include <memory>
#include <cstdlib>

namespace skif::rmlui
{

struct App::Impl
{
    WindowConfig                 config;
    std::vector<std::string>    resource_directories;
    std::unique_ptr<IWindowManager> window_manager;
    std::unique_ptr<IEventLoop>     event_loop;
    std::unique_ptr<IPluginManager> plugin_manager;
    std::unique_ptr<IViewRegistry>  view_registry;
    std::unique_ptr<IViewHost>      view_host;
    std::unique_ptr<InputManagerImpl> input_manager;  // Используем конкретный тип для доступа к внутренним методам

    // RmlUi state
    std::unique_ptr<GladGLContext>  gl;
    std::unique_ptr<Rml::RendererGlad33> render_impl;
    Rml::Context* context = nullptr;

    // Единый контекст окна для GLFW callbacks
    WindowContext window_context;

    // Конфигурация начального view (для B2)
    std::string initial_view_name;
    std::string fallback_rml_path;
    
    // Helper для получения GLFWwindow из IWindow (GLFW-специфичный метод в WindowImpl)
    static GLFWwindow* GetGlfwWindow(IWindow& window)
    {
        return static_cast<WindowImpl&>(window).GetGlfwWindow();
    }
};

App::App(int argc, char* argv[])
    : pimpl_(std::make_unique<Impl>())
{
    // Инициализация по умолчанию
    pimpl_->window_manager = std::make_unique<WindowManagerImpl>();
    pimpl_->event_loop     = std::make_unique<EventLoopImpl>();
    pimpl_->plugin_manager = std::make_unique<PluginManagerImpl>();
    pimpl_->view_registry  = std::make_unique<ViewRegistryImpl>();
    pimpl_->input_manager  = std::make_unique<InputManagerImpl>();
    
    // Подключаем ViewRegistry к PluginManager
    pimpl_->plugin_manager->SetViewRegistry(pimpl_->view_registry.get());
    
    // ViewHost создаётся после инициализации RmlUi контекста
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

IViewRegistry&
App::GetViewRegistry() noexcept
{
    return *pimpl_->view_registry;
}

IViewHost&
App::GetViewHost() noexcept
{
    return *pimpl_->view_host;
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
// Private initialization methods (B1 refactoring)
// ============================================================================

bool
App::InitializeGL(IWindow* window)
{
    window->MakeContextCurrent();
    pimpl_->gl = std::make_unique<GladGLContext>();

    // Загрузка GL с использованием glfwGetProcAddress
    const bool gl_loaded = GLAD_MAKE_VERSION(3, 3) >=
        gladLoadGLContext(pimpl_->gl.get(), (GLADloadfunc)glfwGetProcAddress);

    if (!gl_loaded)
    {
        return false;
    }

    // Настройка OpenGL
    pimpl_->gl->ClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    pimpl_->gl->Enable(GL_BLEND);
    pimpl_->gl->BlendEquation(GL_FUNC_ADD);
    pimpl_->gl->BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const auto [fb_width, fb_height] = window->GetFramebufferSize();
    pimpl_->gl->Viewport(0, 0, fb_width, fb_height);

    return true;
}

bool
App::InitializeRmlUi(IWindow* window)
{
    const auto [fb_width, fb_height] = window->GetFramebufferSize();
    
    // Инициализация RmlUi renderer
    pimpl_->render_impl = std::make_unique<Rml::RendererGlad33>(
        [w = window, gl = pimpl_->gl.get()](void) -> GladGLContext*
        {
            w->MakeContextCurrent();
            return gl;
        }
    );

    Rml::SetRenderInterface(pimpl_->render_impl.get());
    Rml::Initialise();

    // Создание RmlUi контекста
    pimpl_->context = Rml::CreateContext("default", Rml::Vector2i(fb_width, fb_height));
    if (!pimpl_->context)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to create RmlUi context.");
        Rml::Shutdown();
        return false;
    }
    
    // Обновляем rml_context в window_context для callback
    pimpl_->window_context.rml_context = pimpl_->context;

    // Создание ViewHost и привязка к контексту
    pimpl_->view_host = std::make_unique<ViewHostImpl>(*pimpl_->view_registry);
    pimpl_->view_host->SetContext(pimpl_->context);
    
    // Установка RmlUi контекста в InputManager
    pimpl_->input_manager->SetContext(pimpl_->context);

    return true;
}

bool
App::LoadFonts()
{
    // Загрузка шрифтов - сначала из директорий ресурсов, затем по умолчанию
    bool font_loaded = false;
    for (const auto& dir : pimpl_->resource_directories)
    {
        const std::string font_path = dir + "/fonts/IBM_Plex_Mono/IBMPlexMono-Regular.ttf";
        font_loaded = Rml::LoadFontFace(font_path.c_str());
        if (font_loaded)
        {
            break;
        }
    }
    
    // Если не загружен из resource directories, пробуем стандартный путь
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

void
App::SetupGlfwCallbacks(IWindow* window)
{
    GLFWwindow* glfw_window = Impl::GetGlfwWindow(*window);
    
    // Настраиваем WindowContext для GLFW callbacks
    pimpl_->window_context.gl = pimpl_->gl.get();
    pimpl_->window_context.rml_context = pimpl_->context;
    pimpl_->window_context.input_manager = pimpl_->input_manager.get();
    
    // Устанавливаем user pointer для callbacks
    glfwSetWindowUserPointer(glfw_window, &pimpl_->window_context);

    // Callback для изменения размера framebuffer
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

    // Callback для обновления окна (при resize или необходимости перерисовки)
    glfwSetWindowRefreshCallback(
        glfw_window,
        [](GLFWwindow* gw)
        {
            auto* ctx = static_cast<WindowContext*>(glfwGetWindowUserPointer(gw));
            if (!ctx || !ctx->gl || !ctx->rml_context)
            {
                return;
            }
            
            ctx->gl->Clear(GL_COLOR_BUFFER_BIT);
            ctx->rml_context->Update();
            ctx->rml_context->Render();
            glfwSwapBuffers(gw);
        }
    );

    // Инициализация InputManager
    pimpl_->input_manager->SetWindow(glfw_window);
}

void
App::SetupEventLoopCallbacks(IWindow* window)
{
    pimpl_->event_loop->SetShouldCloseCheck(
        [window]() { return window->ShouldClose(); }
    );

    pimpl_->event_loop->OnUpdate(
        [this](float /*dt*/)
        {
            // Обработка GLFW событий
            pimpl_->window_manager->PollEvents();
            
            // Update InputManager
            pimpl_->input_manager->Update();
            
            // Update RmlUi
            if (pimpl_->context)
            {
                pimpl_->context->Update();
            }
        }
    );

    pimpl_->event_loop->OnRender(
        [this, window]()
        {
            // Make context current
            window->MakeContextCurrent();
            
            // Clear the screen
            pimpl_->gl->Clear(GL_COLOR_BUFFER_BIT);
            
            // Render RmlUi
            if (pimpl_->context)
            {
                pimpl_->context->Render();
            }

            // Swap buffers
            window->SwapBuffers();
        }
    );

    pimpl_->event_loop->OnExit(
        [this]()
        {
            // 1. Сначала останавливаем плагины (они могут обращаться к RmlUi)
            pimpl_->plugin_manager->StopPlugins();
            
            // 2. Затем очищаем RmlUi
            Rml::SetRenderInterface(nullptr);
            if (pimpl_->context)
            {
                Rml::RemoveContext(pimpl_->context->GetName());
            }
            Rml::Shutdown();
            pimpl_->render_impl.reset();
        }
    );
}

void
App::StartPluginsAndViews()
{
    // Запуск плагинов (они регистрируют свои view)
    pimpl_->plugin_manager->StartPlugins();

    // Пробуем присоединить и показать initial_view через ViewHost
    if (!pimpl_->initial_view_name.empty())
    {
        pimpl_->view_host->AttachView(pimpl_->initial_view_name, nullptr);
        pimpl_->view_host->ShowView(pimpl_->initial_view_name);
    }

    // Если view не загружен (нет плагина), пробуем fallback RML документ
    if (!pimpl_->view_host->GetActiveView() && !pimpl_->fallback_rml_path.empty())
    {
        Rml::Log::Message(Rml::Log::LT_INFO, "Trying to load fallback RML: %s", pimpl_->fallback_rml_path.c_str());
        auto* document = pimpl_->context->LoadDocument(pimpl_->fallback_rml_path.c_str());
        
        if (document)
        {
            Rml::Log::Message(Rml::Log::LT_INFO, "Successfully loaded fallback RML: %s", pimpl_->fallback_rml_path.c_str());
            document->Show();
        }
        else
        {
            Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to load fallback RML document: %s", pimpl_->fallback_rml_path.c_str());
        }
    }
}

void
App::Cleanup(std::shared_ptr<IWindow>& window)
{
    pimpl_->plugin_manager->Shutdown();
    pimpl_->window_manager->DestroyWindow(window);
    pimpl_->window_manager->Shutdown();
}

// ============================================================================
// Main run method
// ============================================================================

App::Return_Code
App::run()
{
    // Инициализация WindowManager
    if (!pimpl_->window_manager->Initialize())
    {
        return EXIT_FAILURE;
    }

    // Инициализация plugin manager
    if (!pimpl_->plugin_manager->Initialize())
    {
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    // Создание главного окна
    auto window = pimpl_->window_manager->CreateWindow(pimpl_->config);
    if (!window)
    {
        pimpl_->plugin_manager->Shutdown();
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    // Инициализация OpenGL
    if (!InitializeGL(window.get()))
    {
        pimpl_->window_manager->DestroyWindow(window);
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    // Настройка GLFW callbacks (до инициализации RmlUi, чтобы window_context был готов)
    SetupGlfwCallbacks(window.get());

    // Инициализация RmlUi
    if (!InitializeRmlUi(window.get()))
    {
        pimpl_->window_manager->DestroyWindow(window);
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    // Загрузка шрифтов
    LoadFonts();

    // Настройка event loop callbacks
    SetupEventLoopCallbacks(window.get());

    // Запуск плагинов и views
    StartPluginsAndViews();

    // Запуск главного цикла
    pimpl_->event_loop->Run();

    // Очистка
    Cleanup(window);

    return EXIT_SUCCESS;
}

} // namespace skif::rmlui
