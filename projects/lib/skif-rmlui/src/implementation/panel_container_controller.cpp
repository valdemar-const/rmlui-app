#include <implementation/panel_container_controller.hpp>

#include <skif/rmlui/editor/i_split_layout.hpp>
#include <skif/rmlui/editor/i_editor_host.hpp>
#include <skif/rmlui/editor/i_editor_registry.hpp>
#include <skif/rmlui/view/lambda_event_listener.hpp>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Types.h>

namespace skif::rmlui
{

PanelContainerController::PanelContainerController(
    ISplitLayout& layout,
    IEditorHost& editor_host,
    IEditorRegistry& registry,
    const SplitNode* node,
    Rml::Element* container_element,
    std::string instance_id)
    : layout_(layout)
    , editor_host_(editor_host)
    , registry_(registry)
    , node_(node)
    , container_(container_element)
    , instance_id_(std::move(instance_id))
{}

PanelContainerController::~PanelContainerController() = default;

void
PanelContainerController::BindEvents()
{
    if (!container_)
    {
        return;
    }

    // Editor switcher — <select class="editor-switcher">
    // Ищем по data-instance атрибуту для точного соответствия
    Rml::Element* switcher = container_->QuerySelector(
        "select.editor-switcher[data-instance=\"" + instance_id_ + "\"]"
    );

    if (switcher)
    {
        BindEvent(switcher, "change",
            [this](Rml::Event& event) { OnEditorSwitcherChange(event); }
        );
    }

    // Hot corners — привязываем mousedown
    Rml::ElementList hot_corners;
    container_->QuerySelectorAll(hot_corners,
        ".hot-corner[data-instance=\"" + instance_id_ + "\"]"
    );

    for (int i = 0; i < static_cast<int>(hot_corners.size()); ++i)
    {
        Rml::Element* corner = hot_corners[i];
        std::string action = corner->GetAttribute<Rml::String>("data-action", "");

        BindEvent(corner, "mousedown",
            [this, action](Rml::Event& event) { OnHotCornerMouseDown(event, action); }
        );
    }

    // Menu items
    Rml::ElementList menu_items;
    container_->QuerySelectorAll(menu_items, ".menu-item");
    for (int i = 0; i < static_cast<int>(menu_items.size()); ++i)
    {
        Rml::Element* item = menu_items[i];
        BindEvent(item, "click",
            [this](Rml::Event& event) { OnMenuItemClick(event); }
        );
    }

    Rml::Log::Message(Rml::Log::LT_INFO,
        "PanelContainerController: Bound events for instance '%s'.",
        instance_id_.c_str());
}

void
PanelContainerController::UpdateStatusBar()
{
    if (!container_ || !node_)
    {
        return;
    }

    // Получаем editor instance
    IEditor* editor = editor_host_.GetEditor(instance_id_);
    if (!editor)
    {
        return;
    }

    // Получаем status text
    std::string_view status = editor->GetStatusText();

    // Находим status-text элемент внутри контейнера
    Rml::Element* status_element = container_->QuerySelector(".status-text");
    if (status_element)
    {
        if (status.empty())
        {
            status_element->SetInnerRML("Ready");
        }
        else
        {
            status_element->SetInnerRML(Rml::String(status));
        }
    }
}

void
PanelContainerController::OnEditorSwitcherChange(Rml::Event& event)
{
    Rml::Element* select = event.GetTargetElement();
    if (!select)
    {
        return;
    }

    // Получаем выбранное значение
    Rml::String value = select->GetAttribute<Rml::String>("value", "");
    if (value.empty())
    {
        return;
    }

    Rml::Log::Message(Rml::Log::LT_INFO,
        "PanelContainerController: Editor switcher changed to '%s' for instance '%s'.",
        value.c_str(), instance_id_.c_str());

    // Переключаем редактор через ISplitLayout
    layout_.SwitchEditor(node_, value);
}

void
PanelContainerController::OnHotCornerMouseDown(Rml::Event& event, std::string_view action)
{
    if (action == "split")
    {
        // Определяем направление по позиции угла
        // Верхние углы — split
        Rml::Element* corner = event.GetTargetElement();
        if (!corner)
        {
            return;
        }

        // Определяем направление по классу угла
        bool is_left = corner->IsClassSet("hot-corner-tl") || corner->IsClassSet("hot-corner-bl");
        bool is_top = corner->IsClassSet("hot-corner-tl") || corner->IsClassSet("hot-corner-tr");

        // Горизонтальный split если тянем от левого/правого угла
        // Вертикальный split если тянем от верхнего/нижнего угла
        SplitDirection direction = is_left || !is_left
            ? SplitDirection::Horizontal
            : SplitDirection::Vertical;

        // Для простоты: TL/TR → horizontal split, BL/BR → vertical split
        if (is_top)
        {
            direction = SplitDirection::Horizontal;
        }
        else
        {
            direction = SplitDirection::Vertical;
        }

        Rml::Log::Message(Rml::Log::LT_INFO,
            "PanelContainerController: Split requested for instance '%s', direction=%s.",
            instance_id_.c_str(),
            direction == SplitDirection::Horizontal ? "horizontal" : "vertical");

        // Используем тот же тип редактора для новой панели
        layout_.Split(node_, direction, node_->editor_name, 0.5f);
    }
    else if (action == "merge")
    {
        Rml::Log::Message(Rml::Log::LT_INFO,
            "PanelContainerController: Merge requested for instance '%s'.",
            instance_id_.c_str());

        // Merge: нужно найти parent split node
        // Для Phase 2.5 — простая реализация: merge с сохранением текущей панели
        // Полноценный merge с drag на соседнюю панель — в будущем
    }
}

void
PanelContainerController::OnMenuItemClick(Rml::Event& event)
{
    Rml::Element* item = event.GetTargetElement();
    if (!item)
    {
        return;
    }

    Rml::String action = item->GetAttribute<Rml::String>("data-action", "");
    if (action.empty())
    {
        return;
    }

    Rml::Log::Message(Rml::Log::LT_INFO,
        "PanelContainerController: Menu action '%s' for instance '%s'.",
        action.c_str(), instance_id_.c_str());

    // TODO: В будущем — Command System
    // Пока просто логируем
}

} // namespace skif::rmlui
