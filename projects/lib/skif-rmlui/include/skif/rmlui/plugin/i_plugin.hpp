#pragma once

#include <skif/rmlui/config.hpp>

#include <string_view>
#include <string>

namespace skif::rmlui
{

struct Version
{
    int major = 0;
    int minor = 0;
    int patch = 0;

    constexpr Version() noexcept = default;
    constexpr Version(int major, int minor, int patch) noexcept 
        : major(major), minor(minor), patch(patch) {}
};

/**
 * @brief Интерфейс плагина
 * @note Базовый интерфейс для всех плагинов фреймворка
 */
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    /// Получить имя плагина
    [[nodiscard]] virtual std::string_view GetName() const noexcept = 0;

    /// Получить версию плагина
    [[nodiscard]] virtual Version GetVersion() const noexcept = 0;

    /// Получить описание плагина
    [[nodiscard]] virtual std::string_view GetDescription() const noexcept 
    { 
        return {}; 
    }

    /// Вызывается при загрузке плагина
    virtual void OnLoad(class IPluginRegistry& registry) = 0;

    /// Вызывается при выгрузке плагина
    virtual void OnUnload() noexcept = 0;
};

} // namespace skif::rmlui