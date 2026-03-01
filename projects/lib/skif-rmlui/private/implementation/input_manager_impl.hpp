#pragma once

#include <skif/rmlui/input/i_input_manager.hpp>
#include <skif/rmlui/core/signal.hpp>

#include <array>

// Forward declarations
struct GLFWwindow;
namespace Rml { class Context; }

namespace skif::rmlui
{

/**
 * @brief Реализация менеджера ввода
 * @note Содержит внутренние методы (SetWindow, SetContext, Update) и Signal,
 *       которые не экспонируются в публичном IInputManager.
 */
class InputManagerImpl : public IInputManager
{
public:
    InputManagerImpl();
    ~InputManagerImpl() override;
    
    // ========================================================================
    // IInputManager implementation (публичные query-методы)
    // ========================================================================
    
    [[nodiscard]] bool IsKeyDown(KeyCode key) const override;
    [[nodiscard]] bool IsKeyPressed(KeyCode key) const override;
    
    [[nodiscard]] Vector2f GetMousePosition() const override;
    [[nodiscard]] Vector2f GetMouseDelta() const override;
    [[nodiscard]] bool IsMouseButtonDown(MouseButton button) const override;
    [[nodiscard]] float GetMouseWheel() const override;
    
    // ========================================================================
    // Внутренние методы (не в публичном интерфейсе)
    // ========================================================================
    
    /// Установить GLFW окно для обработки ввода
    void SetWindow(GLFWwindow* window);
    
    /// Установить RmlUi контекст для инъекции событий
    void SetContext(Rml::Context* context);
    
    /// Обновить состояние ввода (вызывать каждый кадр)
    void Update();
    
    // ========================================================================
    // RmlUi integration (внутренние методы)
    // ========================================================================
    
    /// Инъекция нажатия клавиши в RmlUi
    void InjectKeyDown(int key, int modifiers);
    
    /// Инъекция отжатия клавиши в RmlUi
    void InjectKeyUp(int key, int modifiers);
    
    /// Инъекция движения мыши в RmlUi
    void InjectMouseMove(int x, int y, int dx, int dy);
    
    /// Инъекция нажатия кнопки мыши в RmlUi
    void InjectMouseDown(int x, int y, int button, int modifiers);
    
    /// Инъекция отжатия кнопки мыши в RmlUi
    void InjectMouseUp(int x, int y, int button, int modifiers);
    
    // ========================================================================
    // Сигналы событий ввода
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

private:
    GLFWwindow* window_ = nullptr;
    Rml::Context* context_ = nullptr;
    
    // Keyboard state
    std::array<bool, 512> key_states_{};
    std::array<bool, 512> key_pressed_{};
    
    // Mouse state
    Vector2f mouse_position_;
    Vector2f mouse_delta_;
    std::array<bool, 8> mouse_button_states_{};
    float mouse_wheel_ = 0.0f;
    float mouse_wheel_delta_ = 0.0f;
    
    // GLFW Callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
};

} // namespace skif::rmlui
