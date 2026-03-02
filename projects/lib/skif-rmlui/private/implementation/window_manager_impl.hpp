#pragma once

#include <skif/rmlui/core/i_window_manager.hpp>

#include <memory>
#include <vector>

namespace skif::rmlui
{

class WindowImpl;

/**
 * @brief Реализация менеджера окон
 */
class WindowManagerImpl final : public IWindowManager
{
  public:

    WindowManagerImpl();
    ~WindowManagerImpl() override;

    // IWindowManager
    [[nodiscard]] bool                                  Initialize() noexcept override;
    void                                                Shutdown() noexcept override;
    [[nodiscard]] std::shared_ptr<IWindow>              CreateWindow(const WindowConfig &config) override;
    void                                                DestroyWindow(std::shared_ptr<IWindow> window) override;
    [[nodiscard]] IWindow                              *GetMainWindow() const noexcept override;
    [[nodiscard]] std::vector<std::shared_ptr<IWindow>> GetWindows() const override;
    void                                                PollEvents() noexcept override;
    void                                                WaitEvents() noexcept override;
    void                                                WaitEvents(double timeout) noexcept override;

  private:

    std::vector<std::shared_ptr<WindowImpl>> windows_;
    bool                                     initialized_ = false;
};

} // namespace skif::rmlui
