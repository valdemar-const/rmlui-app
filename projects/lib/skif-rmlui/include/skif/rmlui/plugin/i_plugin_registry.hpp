#pragma once

#include <skif/rmlui/plugin/i_plugin.hpp>

#include <memory>
#include <string_view>
#include <vector>

namespace skif::rmlui
{

// Forward declarations
class IViewRegistry;
class IEditorRegistry;

/**
 * @brief Интерфейс реестра плагинов
 * @note Используется плагинами для регистрации себя в системе
 *       и доступа к contribution points.
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

    /// Получить реестр представлений (deprecated — используйте GetEditorRegistry)
    [[nodiscard]] virtual IViewRegistry& GetViewRegistry() = 0;

    /// Получить реестр редакторов — contribution point для регистрации Editor из плагинов
    [[nodiscard]] virtual IEditorRegistry& GetEditorRegistry() = 0;
};

} // namespace skif::rmlui
