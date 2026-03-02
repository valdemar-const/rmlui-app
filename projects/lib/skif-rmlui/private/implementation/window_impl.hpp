#pragma once

#include <skif/rmlui/core/i_window.hpp>

#include <GLFW/glfw3.h>

#include <memory>
#include <string>

namespace skif::rmlui
{

/**
 * @brief Реализация окна на базе GLFW
 */
class WindowImpl final : public IWindow
{
  public:

    explicit WindowImpl(const WindowConfig &config);
    ~WindowImpl() override;

    // IWindow
    [[nodiscard]] void    *GetNativeHandle() noexcept override;
    [[nodiscard]] Vector2i GetSize() const noexcept override;
    [[nodiscard]] Vector2i GetFramebufferSize() const noexcept override;
    void                   SetTitle(std::string_view title) noexcept override;
    void                   Close() noexcept override;
    [[nodiscard]] bool     ShouldClose() const noexcept override;
    void                   MakeContextCurrent() noexcept override;
    void                   SwapBuffers() noexcept override;

    // GLFW-специфичный метод (не в публичном IWindow)
    [[nodiscard]] GLFWwindow *GetGlfwWindow() const noexcept;

  private:

    GLFWwindow *window_ = nullptr;
    std::string title_;
};

} // namespace skif::rmlui
