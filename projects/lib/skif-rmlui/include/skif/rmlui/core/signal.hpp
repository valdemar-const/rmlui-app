#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <algorithm>

namespace skif::rmlui
{

/// Handle для отключения подписки на сигнал
class Connection
{
public:
    Connection() = default;
    
    /// Отключить подписку
    void Disconnect()
    {
        if (disconnect_fn_)
        {
            disconnect_fn_();
            disconnect_fn_ = nullptr;
        }
    }
    
    /// Проверить, активна ли подписка
    [[nodiscard]] bool IsConnected() const noexcept
    {
        return disconnect_fn_ != nullptr;
    }

private:
    template<typename...> friend class Signal;
    std::function<void()> disconnect_fn_;
};

/// Сигнал с поддержкой disconnect
template<typename... Args>
class Signal
{
public:
    using Callback = std::function<void(Args...)>;
    
    /// Подключить callback и получить Connection для отключения
    [[nodiscard]] Connection Connect(Callback callback)
    {
        const auto id = next_id_++;
        slots_.push_back({id, std::move(callback)});
        
        Connection conn;
        conn.disconnect_fn_ = [this, id]()
        {
            std::erase_if(slots_, [id](const Slot& s) { return s.id == id; });
        };
        return conn;
    }
    
    /// Вызвать все подключённые callbacks
    void operator()(Args... args) const
    {
        // Копируем на случай модификации во время итерации
        auto slots_copy = slots_;
        for (const auto& slot : slots_copy)
        {
            slot.callback(args...);
        }
    }
    
    /// Отключить все подписки
    void DisconnectAll()
    {
        slots_.clear();
    }
    
    /// Проверить, есть ли подписчики
    [[nodiscard]] bool Empty() const noexcept
    {
        return slots_.empty();
    }
    
    /// Получить количество подписчиков
    [[nodiscard]] std::size_t Size() const noexcept
    {
        return slots_.size();
    }

private:
    struct Slot
    {
        uint64_t id;
        Callback callback;
    };
    
    std::vector<Slot> slots_;
    uint64_t next_id_ = 0;
};

} // namespace skif::rmlui
