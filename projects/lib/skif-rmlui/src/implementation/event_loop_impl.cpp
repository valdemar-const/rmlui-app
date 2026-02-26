#include <implementation/event_loop_impl.hpp>

#include <chrono>

namespace skif::rmlui
{

EventLoopImpl::EventLoopImpl()
{
    last_time_ = std::chrono::steady_clock::now();
}

EventLoopImpl::~EventLoopImpl() = default;

void
EventLoopImpl::Run()
{
    if (running_)
    {
        return;
    }

    running_     = true;
    should_stop_ = false;
    last_time_   = std::chrono::steady_clock::now();

    while (!should_stop_)
    {
        // Проверка завершения
        if (should_close_check_.has_value() && should_close_check_.value()())
        {
            break;
        }

        // Вычисление delta time
        const auto current_time = std::chrono::steady_clock::now();
        const auto elapsed       = std::chrono::duration<float>(current_time - last_time_);
        last_time_               = current_time;

        delta_time_  = elapsed.count();
        total_time_ += delta_time_;

        // Update
        if (update_callback_.has_value())
        {
            update_callback_.value()(delta_time_);
        }

        // Render
        if (render_callback_.has_value())
        {
            render_callback_.value()();
        }
    }

    running_ = false;

    if (exit_callback_.has_value())
    {
        exit_callback_.value()();
    }
}

void
EventLoopImpl::Stop() noexcept
{
    should_stop_ = true;
}

bool
EventLoopImpl::IsRunning() const noexcept
{
    return running_;
}

void
EventLoopImpl::SetFixedDeltaTime(float dt) noexcept
{
    fixed_delta_time_ = dt;
}

float
EventLoopImpl::GetFixedDeltaTime() const noexcept
{
    return fixed_delta_time_;
}

float
EventLoopImpl::GetDeltaTime() const noexcept
{
    return delta_time_;
}

double
EventLoopImpl::GetTotalTime() const noexcept
{
    return total_time_;
}

void
EventLoopImpl::OnUpdate(UpdateCallback callback)
{
    update_callback_ = std::move(callback);
}

void
EventLoopImpl::OnRender(RenderCallback callback)
{
    render_callback_ = std::move(callback);
}

void
EventLoopImpl::OnExit(ExitCallback callback)
{
    exit_callback_ = std::move(callback);
}

void
EventLoopImpl::SetShouldCloseCheck(ShouldCloseCheck check)
{
    should_close_check_ = std::move(check);
}

} // namespace skif::rmlui