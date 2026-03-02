#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/core/i_window_manager.hpp>
#include <skif/rmlui/core/i_event_loop.hpp>
#include <skif/rmlui/plugin/i_plugin_manager.hpp>
#include <skif/rmlui/view/i_view_registry.hpp>
#include <skif/rmlui/view/i_view_host.hpp>
#include <skif/rmlui/input/i_input_manager.hpp>
#include <skif/rmlui/editor/i_editor_registry.hpp>
#include <skif/rmlui/editor/i_editor_host.hpp>
#include <skif/rmlui/editor/i_split_layout.hpp>
#include <skif/rmlui/editor/split_node.hpp>

#include <memory>
#include <string>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Главный класс приложения
 * @note Инкапсулирует инициализацию и запуск приложения.
 *       Предоставляет доступ к Editor Platform через contribution points.
 */
class App
{
  public:

    using Return_Code = int;

    App(int argc, char *argv[]);
    ~App();

    App(const App &)            = delete;
    App &operator=(const App &) = delete;
    App(App &&)                 = delete;
    App &operator=(App &&)      = delete;

    /// Запустить приложение
    Return_Code run();

    // ========================================================================
    // Core
    // ========================================================================

    /// Получить менеджер окон
    [[nodiscard]] IWindowManager &GetWindowManager() noexcept;

    /// Получить менеджер плагинов
    [[nodiscard]] IPluginManager &GetPluginManager() noexcept;

    /// Получить менеджер ввода
    [[nodiscard]] IInputManager &GetInputManager() noexcept;

    /// Получить конфигурацию
    [[nodiscard]] const WindowConfig &GetConfig() const noexcept;
    WindowConfig                     &GetConfig() noexcept;

    // ========================================================================
    // Editor Platform
    // ========================================================================

    /// Получить реестр редакторов — contribution point
    [[nodiscard]] IEditorRegistry &GetEditorRegistry() noexcept;

    /// Получить хост редакторов
    [[nodiscard]] IEditorHost &GetEditorHost() noexcept;

    /// Получить layout
    [[nodiscard]] ISplitLayout &GetSplitLayout() noexcept;

    /// Установить начальный layout (вызывать до run())
    void SetInitialLayout(std::unique_ptr<SplitNode> root);

    // ========================================================================
    // Legacy View System (deprecated)
    // ========================================================================

    /// Получить реестр представлений (deprecated — используйте GetEditorRegistry)
    [[nodiscard]] IViewRegistry &GetViewRegistry() noexcept;

    /// Получить хост представлений (deprecated — используйте GetEditorHost)
    [[nodiscard]] IViewHost &GetViewHost() noexcept;

    // ========================================================================
    // Resources
    // ========================================================================

    /// Добавить директорию ресурсов
    void AddResourceDirectory(std::string_view path);

    /// Получить директории ресурсов
    [[nodiscard]] const std::vector<std::string> &GetResourceDirectories() const noexcept;

    /// Установить начальное view для отображения (deprecated — используйте SetInitialLayout)
    void SetInitialView(std::string_view view_name);

    /// Установить fallback RML документ
    void SetFallbackRml(std::string_view rml_path);

  private:

    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace skif::rmlui
