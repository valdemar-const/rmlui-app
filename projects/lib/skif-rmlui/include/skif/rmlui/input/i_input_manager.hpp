#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/core/math_types.hpp>
#include <skif/rmlui/input/key_codes.hpp>
#include <skif/rmlui/input/mouse_buttons.hpp>

#include <functional>
#include <cstdint>
#include <vector>

// Forward declarations
struct GLFWwindow;
namespace Rml { class Context; }

namespace skif::rmlui
{

/**
 * @brief Сигнал (упрощённая версия)
 */
template<typename... Args>
class Signal
{
public:
    using Callback = std::function<void(Args...)>;
    
    void Connect(Callback callback)
    {
        callbacks_.push_back(std::move(callback));
    }
    
    void operator()(Args... args) const
    {
        for (const auto& callback : callbacks_)
        {
            callback(args...);
        }
    }
    
private:
    std::vector<Callback> callbacks_;
};

/**
 * @brief Интерфейс менеджера ввода
 * @note Управляет клавиатурой, мышью и интеграцией с RmlUi
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
    
    // ========================================================================
    // RmlUi integration
    // ========================================================================
    
    /// Инъекция нажатия клавиши в RmlUi
    virtual void InjectKeyDown(int key, int modifiers) = 0;
    
    /// Инъекция отжатия клавиши в RmlUi
    virtual void InjectKeyUp(int key, int modifiers) = 0;
    
    /// Инъекция движения мыши в RmlUi
    virtual void InjectMouseMove(int x, int y, int dx, int dy) = 0;
    
    /// Инъекция нажатия кнопки мыши в RmlUi
    virtual void InjectMouseDown(int x, int y,
                                 int button,
                                 int modifiers) = 0;
    
    /// Инъекция отжатия кнопки мыши в RmlUi
    virtual void InjectMouseUp(int x, int y,
                               int button,
                               int modifiers) = 0;
    
    // ========================================================================
    // Global input events
    // ========================================================================
    
    /// Сигнал нажатия клавиши
    Signal<KeyCode> OnKeyDown;
    
    /// Сигнал отжатия клавиши
    Signal<KeyCode> OnKeyUp;
    
    /// Сигнал нажатия кнопки мыши
    Signal<MouseButton> OnMouseDown;
    
    /// Сигнал отжатия кнопки мыши
    Signal<MouseButton> OnMouseUp;
    
    /// Сигнал движения мыши
    Signal<Vector2f> OnMouseMove;
    
    // ========================================================================
    // GLFW callbacks setup
    // ========================================================================
    
    /// Установить GLFW окно для обработки ввода
    virtual void SetWindow(struct GLFWwindow* window) = 0;
    
    /// Установить RmlUi контекст для инъекции событий
    virtual void SetContext(Rml::Context* context) = 0;
    
    /// Обновить состояние ввода (вызывать каждый кадр)
    virtual void Update() = 0;
};

} // namespace skif::rmlui