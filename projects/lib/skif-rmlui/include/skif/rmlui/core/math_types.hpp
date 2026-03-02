#pragma once

namespace skif::rmlui
{

/// Вектор 2D с целочисленными координатами
struct Vector2i
{
    int x = 0;
    int y = 0;

    constexpr Vector2i() noexcept = default;

    constexpr Vector2i(int x, int y) noexcept
        : x(x)
        , y(y)
    {
    }
};

/// Вектор 2D с координатами с плавающей точкой
struct Vector2f
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2f() noexcept = default;

    constexpr Vector2f(float x, float y) noexcept
        : x(x)
        , y(y)
    {
    }
};

} // namespace skif::rmlui
