#pragma once

#include <skif/rmlui/config.hpp>

#include <functional>
#include <optional>

namespace skif::rmlui
{

/**
 * @brief Интерфейс главного цикла приложения
 * @note Управляет обновлением и рендерингом
 */
class IEventLoop
{
  public:

    using UpdateCallback   = std::function<void(float delta_time)>;
    using RenderCallback   = std::function<void()>;
    using ExitCallback     = std::function<void()>;
    using ShouldCloseCheck = std::function<bool()>;

    virtual ~IEventLoop() = default;

    /// Запустить главный цикл
    virtual void Run() = 0;

    /// Остановить главный цикл
    virtual void Stop() noexcept = 0;

    /// Проверить, запущен ли цикл
    [[nodiscard]] virtual bool IsRunning() const noexcept = 0;

    /// Установить фиксированный delta time для физики
    virtual void SetFixedDeltaTime(float dt) noexcept = 0;

    /// Получить фиксированный delta time
    [[nodiscard]] virtual float GetFixedDeltaTime() const noexcept = 0;

    /// Получить текущий delta time
    [[nodiscard]] virtual float GetDeltaTime() const noexcept = 0;

    /// Получить общее время работы
    [[nodiscard]] virtual double GetTotalTime() const noexcept = 0;

    /// Установить callback обновления
    virtual void OnUpdate(UpdateCallback callback) = 0;

    /// Установить callback рендеринга
    virtual void OnRender(RenderCallback callback) = 0;

    /// Установить callback выхода
    virtual void OnExit(ExitCallback callback) = 0;

    /// Установить функцию проверки завершения
    virtual void SetShouldCloseCheck(ShouldCloseCheck check) = 0;
};

} // namespace skif::rmlui
