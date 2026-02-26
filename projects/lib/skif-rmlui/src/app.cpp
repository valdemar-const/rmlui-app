#include <skif/rmlui/app.hpp>

#include <skif/rmlui/config.hpp>

#include <implementation/window_manager_impl.hpp>
#include <implementation/event_loop_impl.hpp>
#include <implementation/plugin_manager_impl.hpp>

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
    std::vector<std::string>     resource_directories;
    std::unique_ptr<IWindowManager> window_manager;
    std::unique_ptr<IEventLoop>     event_loop;
    std::unique_ptr<IPluginManager> plugin_manager;

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

    // Загрузка документа
    Rml::ElementDocument* document = pimpl_->context->LoadDocument("assets/ui/basic.rml");
    if (document)
    {
        document->Show();
    }

    // Запуск плагинов
    pimpl_->plugin_manager->StartPlugins();

    // Настройка event loop
    pimpl_->event_loop->SetShouldCloseCheck(
        [window = window.get()]() { return window->ShouldClose(); }
    );

    pimpl_->event_loop->OnUpdate(
        [this](float /*dt*/)
        {
            // Обработка GLFW событий
            pimpl_->window_manager->PollEvents();
            
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
        [this, document]()
        {
            if (document)
            {
                document->Close();
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
