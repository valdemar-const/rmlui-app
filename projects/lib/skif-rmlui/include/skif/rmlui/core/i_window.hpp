#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/core/math_types.hpp>

#include <cstddef>
#include <string>
#include <string_view>

namespace skif::rmlui
{

struct WindowConfig
{
    int         width         = 1280;
    int         height        = 720;
    std::string title         = "SkifRmlUi Application";
    bool        resizable     = true;
    bool        fullscreen    = false;
    bool        vsync         = true;
    int         context_major = 3;
    int         context_minor = 3;
};

/**
 * @brief Публичный интерфейс окна
 * @note Кроссплатформенная абстракция над GLFW окнами.
 *       GLFW-специфичные методы (GetGlfwWindow) находятся в WindowImpl.
 */
class IWindow
{
public:
    virtual ~IWindow() = default;

    /// Получить нативный дескриптор окна (HWND на Windows, Window на X11, etc.)
    [[nodiscard]] virtual void* GetNativeHandle() noexcept = 0;

    /// Получить размер окна в экранных координатах
    [[nodiscard]] virtual Vector2i GetSize() const noexcept = 0;

    /// Получить размер framebuffer (может отличаться от размера окна на HiDPI)
    [[nodiscard]] virtual Vector2i GetFramebufferSize() const noexcept = 0;

    /// Установить заголовок окна
    virtual void SetTitle(std::string_view title) noexcept = 0;

    /// Закрыть окно
    virtual void Close() noexcept = 0;

    /// Проверить, должно ли окно быть закрыто
    [[nodiscard]] virtual bool ShouldClose() const noexcept = 0;

    /// Сделать контекст окна текущим
    virtual void MakeContextCurrent() noexcept = 0;

    /// Обменять буферы (для double buffering)
    virtual void SwapBuffers() noexcept = 0;
};

} // namespace skif::rmlui
