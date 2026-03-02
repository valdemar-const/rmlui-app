#pragma once

#include <skif/rmlui/config.hpp>

namespace skif::rmlui
{

/**
 * @brief Кнопки мыши
 * @note Соответствуют GLFW mouse buttons
 */
enum class MouseButton
{
    Left   = 0,
    Right  = 1,
    Middle = 2,

    // Additional buttons
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7,
};

} // namespace skif::rmlui
