#include <implementation/window_manager_impl.hpp>

#include <implementation/window_impl.hpp>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cassert>

namespace skif::rmlui
{

WindowManagerImpl::WindowManagerImpl() = default;

WindowManagerImpl::~WindowManagerImpl()
{
    Shutdown();
}

bool
WindowManagerImpl::Initialize() noexcept
{
    if (initialized_)
    {
        return true;
    }

    const bool result = glfwInit() == GLFW_TRUE;
    if (result)
    {
        initialized_ = true;
    }

    return result;
}

void
WindowManagerImpl::Shutdown() noexcept
{
    if (!initialized_)
    {
        return;
    }

    // Удаляем все окна
    windows_.clear();

    glfwTerminate();
    initialized_ = false;
}

std::shared_ptr<IWindow>
WindowManagerImpl::CreateWindow(const WindowConfig &config)
{
    assert(initialized_ && "WindowManager must be initialized before creating windows");

    auto window = std::make_shared<WindowImpl>(config);
    windows_.push_back(window);

    return window;
}

void
WindowManagerImpl::DestroyWindow(std::shared_ptr<IWindow> window)
{
    // Удаляем окно из списка
    auto it = std::remove_if(
            windows_.begin(),
            windows_.end(),
            [&window](const std::shared_ptr<WindowImpl> &w)
            {
                return w.get() == window.get();
            }
    );
    windows_.erase(it, windows_.end());
}

IWindow *
WindowManagerImpl::GetMainWindow() const noexcept
{
    if (windows_.empty())
    {
        return nullptr;
    }
    return windows_.front().get();
}

std::vector<std::shared_ptr<IWindow>>
WindowManagerImpl::GetWindows() const
{
    std::vector<std::shared_ptr<IWindow>> result;
    result.reserve(windows_.size());
    for (const auto &w : windows_)
    {
        result.push_back(w);
    }
    return result;
}

void
WindowManagerImpl::PollEvents() noexcept
{
    glfwPollEvents();
}

void
WindowManagerImpl::WaitEvents() noexcept
{
    glfwWaitEvents();
}

void
WindowManagerImpl::WaitEvents(double timeout) noexcept
{
    glfwWaitEventsTimeout(timeout);
}

} // namespace skif::rmlui
