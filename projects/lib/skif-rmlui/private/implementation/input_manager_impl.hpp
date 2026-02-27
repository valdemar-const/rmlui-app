#pragma once

#include <skif/rmlui/input/i_input_manager.hpp>

#include <array>
#include <unordered_map>

// Forward declarations
struct GLFWwindow;
namespace Rml { class Context; }

namespace skif::rmlui
{

/**
 * @brief Реализация менеджера ввода
 */
class InputManagerImpl : public IInputManager
{
public:
    InputManagerImpl();
    ~InputManagerImpl() override;
    
    // IInputManager implementation
    [[nodiscard]] bool IsKeyDown(KeyCode key) const override;
    [[nodiscard]] bool IsKeyPressed(KeyCode key) const override;
    
    [[nodiscard]] Vector2f GetMousePosition() const override;
    [[nodiscard]] Vector2f GetMouseDelta() const override;
    [[nodiscard]] bool IsMouseButtonDown(MouseButton button) const override;
    [[nodiscard]] float GetMouseWheel() const override;
    
    void InjectKeyDown(int key, int modifiers) override;
    void InjectKeyUp(int key, int modifiers) override;
    void InjectMouseMove(int x, int y, int dx, int dy) override;
    void InjectMouseDown(int x, int y, int button, int modifiers) override;
    void InjectMouseUp(int x, int y, int button, int modifiers) override;
    
    void SetWindow(GLFWwindow* window) override;
    void SetContext(Rml::Context* context) override;
    void Update() override;

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
    
    // Callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
};

} // namespace skif::rmlui