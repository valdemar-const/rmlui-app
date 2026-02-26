#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/core/i_window_manager.hpp>
#include <skif/rmlui/core/i_event_loop.hpp>
#include <skif/rmlui/plugin/i_plugin_manager.hpp>
#include <skif/rmlui/view/i_view_registry.hpp>
#include <skif/rmlui/view/i_view_host.hpp>

#include <memory>
#include <string>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Главный класс приложения
 * @note Инкапсулирует инициализацию и запуск приложения
 */
class App
{
public:
    using Return_Code = int;

    App(int argc, char* argv[]);
    ~App();

    App(const App&)            = delete;
    App& operator=(const App&) = delete;
    App(App&&)                 = delete;
    App& operator=(App&&)      = delete;

    /// Запустить приложение
    Return_Code run();

    /// Получить менеджер окон
    [[nodiscard]] IWindowManager& GetWindowManager() noexcept;

    /// Получить менеджер плагинов
    [[nodiscard]] IPluginManager& GetPluginManager() noexcept;

    /// Получить реестр представлений
    [[nodiscard]] IViewRegistry& GetViewRegistry() noexcept;

    /// Получить хост представлений
    [[nodiscard]] IViewHost& GetViewHost() noexcept;

    /// Получить конфигурацию
    [[nodiscard]] const WindowConfig& GetConfig() const noexcept;
    WindowConfig& GetConfig() noexcept;

    /// Добавить директорию ресурсов
    void AddResourceDirectory(std::string_view path);

    /// Получить директории ресурсов
    [[nodiscard]] const std::vector<std::string>& GetResourceDirectories() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace skif::rmlui
