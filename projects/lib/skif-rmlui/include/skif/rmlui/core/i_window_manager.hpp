#pragma once

#include <skif/rmlui/core/i_window.hpp>

#include <memory>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Интерфейс менеджера окон
 * @note Управляет созданием и жизненным циклом окон
 */
class IWindowManager
{
  public:

    virtual ~IWindowManager() = default;

    /// Инициализировать GLFW (вызывается один раз при старте приложения)
    [[nodiscard]] virtual bool Initialize() noexcept = 0;

    /// Завершить работу с GLFW (вызывается при завершении приложения)
    virtual void Shutdown() noexcept = 0;

    /// Создать новое окно
    [[nodiscard]] virtual std::shared_ptr<IWindow> CreateWindow(const WindowConfig &config) = 0;

    /// Удалить окно
    virtual void DestroyWindow(std::shared_ptr<IWindow> window) = 0;

    /// Получить главное окно
    [[nodiscard]] virtual IWindow *GetMainWindow() const noexcept = 0;

    /// Получить все окна
    [[nodiscard]] virtual std::vector<std::shared_ptr<IWindow>> GetWindows() const = 0;

    /// Опросить события (glfwPollEvents)
    virtual void PollEvents() noexcept = 0;

    /// Ожидать события (glfwWaitEvents)
    virtual void WaitEvents() noexcept = 0;

    /// Ожидать события с таймаутом (glfwWaitEventsTimeout)
    virtual void WaitEvents(double timeout) noexcept = 0;
};

} // namespace skif::rmlui
