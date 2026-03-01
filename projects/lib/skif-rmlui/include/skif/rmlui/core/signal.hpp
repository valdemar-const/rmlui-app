#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>

namespace skif::rmlui
{

/// Handle для отключения подписки на сигнал.
/// Безопасен после уничтожения Signal — использует weak_ptr.
class Connection
{
public:
    Connection() = default;
    
    /// Отключить подписку
    void Disconnect()
    {
        if (auto state = state_.lock())
        {
            if (disconnect_fn_)
            {
                disconnect_fn_(state);
                disconnect_fn_ = nullptr;
            }
        }
        else
        {
            // Signal уже уничтожен — просто очищаем
            disconnect_fn_ = nullptr;
        }
    }
    
    /// Проверить, активна ли подписка
    [[nodiscard]] bool IsConnected() const noexcept
    {
        if (!disconnect_fn_)
        {
            return false;
        }
        return !state_.expired();
    }

private:
    template<typename...> friend class Signal;
    
    struct StateBase
    {
        virtual ~StateBase() = default;
    };
    
    using DisconnectFn = std::function<void(std::shared_ptr<StateBase>&)>;
    
    std::weak_ptr<StateBase> state_;
    DisconnectFn disconnect_fn_;
};

/// Сигнал с поддержкой безопасного disconnect.
/// Connection захватывает weak_ptr на внутреннее состояние,
/// поэтому безопасен после уничтожения или перемещения Signal.
template<typename... Args>
class Signal
{
public:
    using Callback = std::function<void(Args...)>;
    
    Signal() : state_(std::make_shared<State>()) {}
    
    // Некопируемый, но перемещаемый
    Signal(const Signal&) = delete;
    Signal& operator=(const Signal&) = delete;
    Signal(Signal&&) noexcept = default;
    Signal& operator=(Signal&&) noexcept = default;
    
    /// Подключить callback и получить Connection для отключения
    [[nodiscard]] Connection Connect(Callback callback)
    {
        const auto id = state_->next_id++;
        state_->slots.push_back({id, std::move(callback)});
        
        Connection conn;
        conn.state_ = state_;
        conn.disconnect_fn_ = [id](std::shared_ptr<Connection::StateBase>& base_state)
        {
            auto* state = static_cast<State*>(base_state.get());
            std::erase_if(state->slots, [id](const Slot& s) { return s.id == id; });
        };
        return conn;
    }
    
    /// Вызвать все подключённые callbacks
    void operator()(Args... args) const
    {
        // Копируем на случай модификации во время итерации
        auto slots_copy = state_->slots;
        for (const auto& slot : slots_copy)
        {
            slot.callback(args...);
        }
    }
    
    /// Отключить все подписки
    void DisconnectAll()
    {
        state_->slots.clear();
    }
    
    /// Проверить, есть ли подписчики
    [[nodiscard]] bool Empty() const noexcept
    {
        return state_->slots.empty();
    }
    
    /// Получить количество подписчиков
    [[nodiscard]] std::size_t Size() const noexcept
    {
        return state_->slots.size();
    }

private:
    struct Slot
    {
        uint64_t id;
        Callback callback;
    };
    
    struct State : Connection::StateBase
    {
        std::vector<Slot> slots;
        uint64_t next_id = 0;
    };
    
    std::shared_ptr<State> state_;
};

} // namespace skif::rmlui
