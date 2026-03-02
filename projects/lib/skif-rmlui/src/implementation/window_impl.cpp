#include <implementation/window_impl.hpp>

#include <GLFW/glfw3.h>

#include <cassert>

namespace skif::rmlui
{

WindowImpl::WindowImpl(const WindowConfig &config)
{
    // Настройка GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config.context_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config.context_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // Для macOS
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    // Создание окна
    GLFWmonitor *monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    window_              = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);

    if (!window_)
    {
        // В реальном приложении здесь нужна обработка ошибок
        return;
    }

    title_ = config.title;
}

WindowImpl::~WindowImpl()
{
    if (window_)
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
}

void *
WindowImpl::GetNativeHandle() noexcept
{
    return window_;
}

Vector2i
WindowImpl::GetSize() const noexcept
{
    int width  = 0;
    int height = 0;
    glfwGetWindowSize(window_, &width, &height);
    return {width, height};
}

Vector2i
WindowImpl::GetFramebufferSize() const noexcept
{
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(window_, &width, &height);
    return {width, height};
}

void
WindowImpl::SetTitle(std::string_view title) noexcept
{
    title_ = title;
    glfwSetWindowTitle(window_, title_.c_str());
}

void
WindowImpl::Close() noexcept
{
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
}

bool
WindowImpl::ShouldClose() const noexcept
{
    return glfwWindowShouldClose(window_) != 0;
}

void
WindowImpl::MakeContextCurrent() noexcept
{
    glfwMakeContextCurrent(window_);
}

void
WindowImpl::SwapBuffers() noexcept
{
    glfwSwapBuffers(window_);
}

GLFWwindow *
WindowImpl::GetGlfwWindow() const noexcept
{
    return window_;
}

} // namespace skif::rmlui
