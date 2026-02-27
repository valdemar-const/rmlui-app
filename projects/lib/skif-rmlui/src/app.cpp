#include <skif/rmlui/app.hpp>

#include <skif/rmlui/config.hpp>

#include <implementation/window_manager_impl.hpp>
#include <implementation/event_loop_impl.hpp>
#include <implementation/plugin_manager_impl.hpp>
#include <implementation/view_registry_impl.hpp>
#include <implementation/view_host_impl.hpp>
#include <implementation/input_manager_impl.hpp>

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
    std::unique_ptr<IInputManager>  input_manager;

    // RmlUi state
    std::unique_ptr<GladGLContext>  gl;
    std::unique_ptr<Rml::RendererGlad33> render_impl;
    Rml::Context* context = nullptr;
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
    window->MakeContextCurrent();
    pimpl_->gl = std::make_unique<GladGLContext>();

    // Загрузка GL с использованием glfwGetProcAddress
    const bool gl_loaded = GLAD_MAKE_VERSION(3, 3) >=
        gladLoadGLContext(pimpl_->gl.get(), (GLADloadfunc)glfwGetProcAddress);

    if (!gl_loaded)
    {
        pimpl_->window_manager->DestroyWindow(window);
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }

    // Настройка OpenGL
    pimpl_->gl->ClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    pimpl_->gl->Enable(GL_BLEND);
    pimpl_->gl->BlendEquation(GL_FUNC_ADD);
    pimpl_->gl->BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    const auto [fb_width, fb_height] = window->GetFramebufferSize();
    pimpl_->gl->Viewport(0, 0, fb_width, fb_height);

    // Структура для передачи состояния в callbacks
    struct WindowState
    {
        GladGLContext* gl      = nullptr;
        Rml::Context*  context = nullptr;
    };
    
    WindowState window_state;
    window_state.gl      = pimpl_->gl.get();
    window_state.context = pimpl_->context; // пока nullptr, будет обновлён после создания контекста
    
    // Устанавливаем user pointer для callbacks
    glfwSetWindowUserPointer(window->GetGlfwWindow(), &window_state);

    // Callback для изменения размера framebuffer
    glfwSetFramebufferSizeCallback(
        window->GetGlfwWindow(),
        [](GLFWwindow* glfw_window, int new_width, int new_height)
        {
            auto* state = static_cast<WindowState*>(glfwGetWindowUserPointer(glfw_window));
            if (!state || !state->gl || !state->context)
            {
                return;
            }
            
            state->gl->Viewport(0, 0, new_width, new_height);
            state->context->SetDimensions({new_width, new_height});
        }
    );

    // Callback для обновления окна (при resize или необходимости перерисовки)
    glfwSetWindowRefreshCallback(
        window->GetGlfwWindow(),
        [](GLFWwindow* glfw_window)
        {
            auto* state = static_cast<WindowState*>(glfwGetWindowUserPointer(glfw_window));
            if (!state || !state->gl || !state->context)
            {
                return;
            }
            
            state->gl->Clear(GL_COLOR_BUFFER_BIT);
            state->context->Update();
            state->context->Render();
            glfwSwapBuffers(glfw_window);
        }
    );

    // Инициализация InputManager
    pimpl_->input_manager->SetWindow(window->GetGlfwWindow());

    // Инициализация RmlUi
    pimpl_->render_impl = std::make_unique<Rml::RendererGlad33>(
        [w = window.get(), gl = pimpl_->gl.get()](void) -> GladGLContext*
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
        pimpl_->window_manager->DestroyWindow(window);
        pimpl_->window_manager->Shutdown();
        return EXIT_FAILURE;
    }
    
    // Обновляем context в window_state для callback
    window_state.context = pimpl_->context;

    // Создание ViewHost и привязка к контексту
    pimpl_->view_host = std::make_unique<ViewHostImpl>(*pimpl_->view_registry);
    pimpl_->view_host->SetContext(pimpl_->context);
    
    // Установка RmlUi контекста в InputManager
    pimpl_->input_manager->SetContext(pimpl_->context);

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

    // Запуск плагинов (они регистрируют свои view)
    pimpl_->plugin_manager->StartPlugins();

    // Пробуем присоединить и показать sample_panel через ViewHost
    // Сначала Attach (создаёт документ), потом Show (показывает его)
    pimpl_->view_host->AttachView("sample_panel", nullptr);
    pimpl_->view_host->ShowView("sample_panel");

    // Если view не загружен (нет плагина), пробуем базовый документ
    if (!pimpl_->view_host->GetActiveView())
    {
        const char* rml_paths[] = {
            "assets/ui/basic.rml",
            "projects/bin/rmlui-app/assets/ui/basic.rml",
            "../projects/bin/rmlui-app/assets/ui/basic.rml"
        };
        
        Rml::ElementDocument* document = nullptr;
        for (const auto* path : rml_paths)
        {
            Rml::Log::Message(Rml::Log::LT_INFO, "Trying to load RML: %s", path);
            document = pimpl_->context->LoadDocument(path);
            if (document)
            {
                Rml::Log::Message(Rml::Log::LT_INFO, "Successfully loaded RML: %s", path);
                break;
            }
        }
        
        if (document)
        {
            document->Show();
        }
        else
        {
            Rml::Log::Message(Rml::Log::LT_ERROR, "Failed to load RML document from all paths!");
        }
    }

    // Настройка event loop
    pimpl_->event_loop->SetShouldCloseCheck(
        [window = window.get()]() { return window->ShouldClose(); }
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
        [this, window = window.get()]()
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
            // Закрываем активный view через ViewHost
            auto* active_view = pimpl_->view_host->GetActiveView();
            if (active_view)
            {
                // ViewHost управляет закрытием документа
            }
            
            Rml::SetRenderInterface(nullptr);
            if (pimpl_->context)
            {
                Rml::RemoveContext(pimpl_->context->GetName());
            }
            Rml::Shutdown();
            pimpl_->render_impl.reset();
        }
    );

    // Запуск главного цикла
    pimpl_->event_loop->Run();

    // Очистка
    pimpl_->plugin_manager->Shutdown();
    pimpl_->window_manager->DestroyWindow(window);
    pimpl_->window_manager->Shutdown();

    return EXIT_SUCCESS;
}

} // namespace skif::rmlui
