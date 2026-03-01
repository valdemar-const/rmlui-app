#pragma once

#include <skif/rmlui/plugin/i_plugin_manager.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
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
 * @brief Реализация менеджера плагинов
 * @note Поддержка статических плагинов (пока без динамической загрузки)
 */
class PluginManagerImpl final : public IPluginManager
{
public:
    PluginManagerImpl() = default;
    ~PluginManagerImpl() override;

    /// Установить реестр представлений (deprecated)
    void SetViewRegistry(IViewRegistry* registry) override;

    /// Установить реестр редакторов
    void SetEditorRegistry(IEditorRegistry* registry);

    // IPluginManager
    [[nodiscard]] bool Initialize() noexcept override;
    void Shutdown() noexcept override;
    [[nodiscard]] bool LoadPlugin(std::string_view path) override;
    void UnloadPlugin(std::string_view name) override;
    void StartPlugins() override;
    void StopPlugins() noexcept override;

    // IPluginRegistry
    void RegisterPlugin(std::unique_ptr<IPlugin> plugin) override;
    [[nodiscard]] IPlugin* GetPlugin(std::string_view name) const override;
    [[nodiscard]] std::vector<IPlugin*> GetPlugins() const override;
    [[nodiscard]] IViewRegistry& GetViewRegistry() override;
    [[nodiscard]] IEditorRegistry& GetEditorRegistry() override;

private:
    struct PluginEntry
    {
        std::unique_ptr<IPlugin> plugin;
        bool started = false;
    };

    std::unordered_map<std::string, PluginEntry> plugins_;
    IViewRegistry* view_registry_ = nullptr;
    IEditorRegistry* editor_registry_ = nullptr;
    bool initialized_ = false;
};

} // namespace skif::rmlui
