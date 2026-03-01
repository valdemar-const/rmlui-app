#pragma once

#include <skif/rmlui/editor/split_node.hpp>

#include <string>
#include <string_view>

// Forward declaration RmlUi
namespace Rml
{
class Element;
class Event;
} // namespace Rml

namespace skif::rmlui
{

class ISplitLayout;
class IEditorHost;
class IEditorRegistry;

/**
 * @brief Контроллер UI панели — управляет header bar, editor switcher, hot corners
 * @note Привязывает event listeners к элементам panel container после загрузки layout документа.
 *       Один контроллер на каждый leaf-узел SplitLayout.
 */
class PanelContainerController
{
public:
    PanelContainerController(
        ISplitLayout& layout,
        IEditorHost& editor_host,
        IEditorRegistry& registry,
        const SplitNode* node,
        Rml::Element* container_element,
        std::string instance_id
    );

    ~PanelContainerController();

    /// Привязать event listeners к элементам panel container
    void BindEvents();

    /// Обновить status bar текст из IEditor::GetStatusText()
    void UpdateStatusBar();

    /// Получить instance_id
    [[nodiscard]] const std::string& GetInstanceId() const noexcept { return instance_id_; }

    /// Получить связанный SplitNode
    [[nodiscard]] const SplitNode* GetNode() const noexcept { return node_; }

private:
    /// Обработчик смены редактора в dropdown
    void OnEditorSwitcherChange(Rml::Event& event);

    /// Обработчик mousedown на hot corner
    void OnHotCornerMouseDown(Rml::Event& event, std::string_view action);

    /// Обработчик клика по menu item
    void OnMenuItemClick(Rml::Event& event);

    ISplitLayout&    layout_;
    IEditorHost&     editor_host_;
    IEditorRegistry& registry_;
    const SplitNode* node_;
    Rml::Element*    container_;
    std::string      instance_id_;
};

} // namespace skif::rmlui
