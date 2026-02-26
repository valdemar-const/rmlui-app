#pragma once

#include <skif/rmlui/config.hpp>

#include <cstddef>
#include <string>
#include <string_view>

struct GLFWwindow;

namespace skif::rmlui
{

struct Vector2i
{
    int x = 0;
    int y = 0;

    constexpr Vector2i() noexcept = default;
    constexpr Vector2i(int x, int y) noexcept : x(x), y(y) {}
};

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
 * @brief Интерфейс окна
 * @note Кроссплатформенная абстракция над GLFW окнами
 */
class IWindow
{
public:
    virtual ~IWindow() = default;

    /// Получить нативный дескриптор окна
    [[nodiscard]] virtual void* GetNativeHandle() noexcept = 0;

    /// Получить размер окна в экранных координатах
    [[nodiscard]] virtual Vector2i GetSize() const noexcept = 0;

    /// Получить размер framebuffer
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

    /// Получить указатель на GLFWwindow (для внутреннего использования)
    [[nodiscard]] virtual GLFWwindow* GetGlfwWindow() const noexcept = 0;

    // Events - будут добавлены позже с использованием Signal
};

} // namespace skif::rmlui