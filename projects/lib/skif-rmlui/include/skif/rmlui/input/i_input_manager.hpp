#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/core/math_types.hpp>
#include <skif/rmlui/input/key_codes.hpp>
#include <skif/rmlui/input/mouse_buttons.hpp>

namespace skif::rmlui
{

/**
 * @brief Публичный интерфейс менеджера ввода
 * @note Содержит только query-методы для пользователей фреймворка.
 *       Внутренние методы (SetWindow, SetContext, Update) находятся в InputManagerImpl.
 */
class IInputManager
{
  public:

    virtual ~IInputManager() = default;

    // ========================================================================
    // Keyboard
    // ========================================================================

    /// Проверить, нажата ли клавиша
    [[nodiscard]] virtual bool IsKeyDown(KeyCode key) const = 0;

    /// Проверить, была ли клавиша нажата в текущем кадре
    [[nodiscard]] virtual bool IsKeyPressed(KeyCode key) const = 0;

    // ========================================================================
    // Mouse
    // ========================================================================

    /// Получить позицию мыши
    [[nodiscard]] virtual Vector2f GetMousePosition() const = 0;

    /// Получить смещение мыши с прошлого кадра
    [[nodiscard]] virtual Vector2f GetMouseDelta() const = 0;

    /// Проверить, нажата ли кнопка мыши
    [[nodiscard]] virtual bool IsMouseButtonDown(MouseButton button) const = 0;

    /// Получить значение колеса мыши
    [[nodiscard]] virtual float GetMouseWheel() const = 0;
};

} // namespace skif::rmlui
