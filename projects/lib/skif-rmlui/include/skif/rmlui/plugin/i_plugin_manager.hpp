#pragma once

#include <skif/rmlui/plugin/i_plugin.hpp>
#include <skif/rmlui/plugin/i_plugin_registry.hpp>

#include <memory>
#include <string_view>
#include <vector>

// Forward declarations
namespace skif::rmlui
{
class IViewRegistry;
class IEditorRegistry;
} // namespace skif::rmlui

namespace skif::rmlui
{

/**
 * @brief Интерфейс менеджера плагинов
 * @note Управляет загрузкой, выгрузкой и жизненным циклом плагинов
 */
class IPluginManager : public IPluginRegistry
{
public:
    /// Установить реестр представлений (deprecated — используйте SetEditorRegistry)
    virtual void SetViewRegistry(IViewRegistry* registry) = 0;

    /// Установить реестр редакторов
    virtual void SetEditorRegistry(IEditorRegistry* registry) = 0;

    /// Инициализировать менеджер плагинов
    virtual bool Initialize() noexcept = 0;

    /// Завершить работу менеджера плагинов
    virtual void Shutdown() noexcept = 0;

    /// Загрузить плагин из DLL/SO
    [[nodiscard]] virtual bool LoadPlugin(std::string_view path) = 0;

    /// Выгрузить плагин по имени
    virtual void UnloadPlugin(std::string_view name) = 0;

    /// Запустить все загруженные плагины
    virtual void StartPlugins() = 0;

    /// Остановить все плагины
    virtual void StopPlugins() noexcept = 0;
};

} // namespace skif::rmlui
