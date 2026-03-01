#pragma once

#include <skif/rmlui/plugin/i_plugin.hpp>
#include <skif/rmlui/view/i_view_registry.hpp>

#include <memory>
#include <string_view>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Интерфейс реестра плагинов
 * @note Используется плагинами для регистрации себя в системе
 */
class IPluginRegistry
{
public:
    virtual ~IPluginRegistry() = default;

    /// Зарегистрировать статический плагин
    virtual void RegisterPlugin(std::unique_ptr<IPlugin> plugin) = 0;

    /// Получить плагин по имени
    [[nodiscard]] virtual IPlugin* GetPlugin(std::string_view name) const = 0;

    /// Получить список всех зарегистрированных плагинов
    [[nodiscard]] virtual std::vector<IPlugin*> GetPlugins() const = 0;

    /// Получить реестр представлений для регистрации view плагинами
    [[nodiscard]] virtual IViewRegistry& GetViewRegistry() = 0;
};

} // namespace skif::rmlui