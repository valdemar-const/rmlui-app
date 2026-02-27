#include <implementation/input_manager_impl.hpp>

#include <GLFW/glfw3.h>
#include <RmlUi/Core/Context.h>

#include <cstring>

namespace skif::rmlui
{

InputManagerImpl::InputManagerImpl()
{
    key_states_.fill(false);
    key_pressed_.fill(false);
    mouse_button_states_.fill(false);
}

InputManagerImpl::~InputManagerImpl() = default;

void
InputManagerImpl::SetWindow(GLFWwindow* window)
{
    if (window_ == window)
    {
        return;
    }
    
    // Disconnect from old window
    if (window_)
    {
        glfwSetKeyCallback(window_, nullptr);
        glfwSetMouseButtonCallback(window_, nullptr);
        glfwSetCursorPosCallback(window_, nullptr);
        glfwSetScrollCallback(window_, nullptr);
    }
    
    window_ = window;
    
    // Connect to new window
    if (window_)
    {
        glfwSetKeyCallback(window_, KeyCallback);
        glfwSetMouseButtonCallback(window_, MouseButtonCallback);
        glfwSetCursorPosCallback(window_, MouseMoveCallback);
        glfwSetScrollCallback(window_, MouseWheelCallback);
        
        // Store this pointer in user pointer for callbacks
        glfwSetWindowUserPointer(window_, this);
    }
}

void
InputManagerImpl::Update()
{
    // Reset pressed keys (they're only valid for one frame)
    key_pressed_.fill(false);
    mouse_wheel_delta_ = mouse_wheel_;
    mouse_wheel_ = 0.0f;
    
    // Reset mouse delta
    mouse_delta_ = {0.0f, 0.0f};
}

// ============================================================================
// Keyboard
// ============================================================================

bool
InputManagerImpl::IsKeyDown(KeyCode key) const
{
    const auto key_int = static_cast<int>(key);
    if (key_int >= 0 && key_int < static_cast<int>(key_states_.size()))
    {
        return key_states_[key_int];
    }
    return false;
}

bool
InputManagerImpl::IsKeyPressed(KeyCode key) const
{
    const auto key_int = static_cast<int>(key);
    if (key_int >= 0 && key_int < static_cast<int>(key_pressed_.size()))
    {
        return key_pressed_[key_int];
    }
    return false;
}

// ============================================================================
// Mouse
// ============================================================================

Vector2f
InputManagerImpl::GetMousePosition() const
{
    return mouse_position_;
}

Vector2f
InputManagerImpl::GetMouseDelta() const
{
    return mouse_delta_;
}

bool
InputManagerImpl::IsMouseButtonDown(MouseButton button) const
{
    const auto button_int = static_cast<int>(button);
    if (button_int >= 0 && button_int < static_cast<int>(mouse_button_states_.size()))
    {
        return mouse_button_states_[button_int];
    }
    return false;
}

float
InputManagerImpl::GetMouseWheel() const
{
    return mouse_wheel_delta_;
}

// ============================================================================
// RmlUi integration
// ============================================================================

void
InputManagerImpl::SetContext(Rml::Context* context)
{
    context_ = context;
}

void
InputManagerImpl::InjectKeyDown(int key, int modifiers)
{
    if (context_)
    {
        context_->ProcessKeyDown(static_cast<Rml::Input::KeyIdentifier>(key), modifiers);
    }
}

void
InputManagerImpl::InjectKeyUp(int key, int modifiers)
{
    if (context_)
    {
        context_->ProcessKeyUp(static_cast<Rml::Input::KeyIdentifier>(key), modifiers);
    }
}

void
InputManagerImpl::InjectMouseMove(int x, int y, int dx, int dy)
{
    if (context_)
    {
        // Use 0 as modifier state - could be tracked separately
        context_->ProcessMouseMove(x, y, 0);
    }
}

void
InputManagerImpl::InjectMouseDown(int x, int y, int button, int modifiers)
{
    if (context_)
    {
        context_->ProcessMouseButtonDown(button, modifiers);
    }
}

void
InputManagerImpl::InjectMouseUp(int x, int y, int button, int modifiers)
{
    if (context_)
    {
        context_->ProcessMouseButtonUp(button, modifiers);
    }
}

// ============================================================================
// GLFW Callbacks
// ============================================================================

void
InputManagerImpl::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto* self = static_cast<InputManagerImpl*>(glfwGetWindowUserPointer(window));
    if (!self)
    {
        return;
    }
    
    if (key >= 0 && key < 512)
    {
        if (action == GLFW_PRESS)
        {
            self->key_states_[key] = true;
            self->key_pressed_[key] = true;
            self->OnKeyDown(static_cast<KeyCode>(key));
            self->InjectKeyDown(key, mods);
        }
        else if (action == GLFW_RELEASE)
        {
            self->key_states_[key] = false;
            self->OnKeyUp(static_cast<KeyCode>(key));
            self->InjectKeyUp(key, mods);
        }
    }
}

void
InputManagerImpl::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto* self = static_cast<InputManagerImpl*>(glfwGetWindowUserPointer(window));
    if (!self)
    {
        return;
    }
    
    if (button >= 0 && button < 8)
    {
        if (action == GLFW_PRESS)
        {
            self->mouse_button_states_[button] = true;
            self->OnMouseDown(static_cast<MouseButton>(button));
            self->InjectMouseDown(static_cast<int>(self->mouse_position_.x),
                                  static_cast<int>(self->mouse_position_.y),
                                  button, mods);
        }
        else if (action == GLFW_RELEASE)
        {
            self->mouse_button_states_[button] = false;
            self->OnMouseUp(static_cast<MouseButton>(button));
            self->InjectMouseUp(static_cast<int>(self->mouse_position_.x),
                                static_cast<int>(self->mouse_position_.y),
                                button, mods);
        }
    }
}

void
InputManagerImpl::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    auto* self = static_cast<InputManagerImpl*>(glfwGetWindowUserPointer(window));
    if (!self)
    {
        return;
    }
    
    const float new_x = static_cast<float>(xpos);
    const float new_y = static_cast<float>(ypos);
    
    self->mouse_delta_.x = new_x - self->mouse_position_.x;
    self->mouse_delta_.y = new_y - self->mouse_position_.y;
    self->mouse_position_.x = new_x;
    self->mouse_position_.y = new_y;
    
    self->OnMouseMove(self->mouse_position_);
    self->InjectMouseMove(static_cast<int>(new_x), static_cast<int>(new_y),
                          static_cast<int>(self->mouse_delta_.x),
                          static_cast<int>(self->mouse_delta_.y));
}

void
InputManagerImpl::MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto* self = static_cast<InputManagerImpl*>(glfwGetWindowUserPointer(window));
    if (!self)
    {
        return;
    }
    
    self->mouse_wheel_ += static_cast<float>(yoffset);
}

} // namespace skif::rmlui