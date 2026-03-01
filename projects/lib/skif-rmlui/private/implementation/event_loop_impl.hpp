#pragma once

#include <skif/rmlui/core/i_event_loop.hpp>

#include <chrono>
#include <optional>

namespace skif::rmlui
{

/**
 * @brief Реализация главного цикла приложения
 */
class EventLoopImpl final : public IEventLoop
{
public:
    EventLoopImpl();
    ~EventLoopImpl() override;

    // IEventLoop
    void Run() override;
    void Stop() noexcept override;
    [[nodiscard]] bool IsRunning() const noexcept override;
    void SetFixedDeltaTime(float dt) noexcept override;
    [[nodiscard]] float GetFixedDeltaTime() const noexcept override;
    [[nodiscard]] float GetDeltaTime() const noexcept override;
    [[nodiscard]] double GetTotalTime() const noexcept override;
    void OnUpdate(UpdateCallback callback) override;
    void OnRender(RenderCallback callback) override;
    void OnExit(ExitCallback callback) override;
    void SetShouldCloseCheck(ShouldCloseCheck check) override;

private:
    bool running_ = false;
    bool should_stop_ = false;

    float fixed_delta_time_ = 1.0f / 60.0f; // 60 FPS по умолчанию
    float delta_time_ = 0.0f;
    double total_time_ = 0.0;

    std::optional<UpdateCallback>   update_callback_;
    std::optional<RenderCallback>   render_callback_;
    std::optional<ExitCallback>     exit_callback_;
    std::optional<ShouldCloseCheck> should_close_check_;

    std::chrono::steady_clock::time_point last_time_;
};

} // namespace skif::rmlui