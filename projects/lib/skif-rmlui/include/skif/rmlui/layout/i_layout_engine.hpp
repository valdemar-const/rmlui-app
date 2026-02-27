#pragma once

#include <skif/rmlui/layout/layout_node.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

// Forward declaration RmlUi
namespace Rml
{
class Element;
class Context;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Интерфейс движка раскладки
 * @note Аналог системы панелей Blender - split, drag-and-drop "горячие углы"
 */
class ILayoutEngine
{
public:
    virtual ~ILayoutEngine() = default;

    /// Установить RmlUi контекст
    virtual void SetContext(Rml::Context* context) = 0;

    /// Создать корневую панель
    virtual void SetRoot(std::unique_ptr<LayoutNode> root) = 0;

    /// Получить корневой узел
    [[nodiscard]] virtual const LayoutNode* GetRoot() const = 0;

    /// Разделить панель
    virtual bool SplitPanel(
        Rml::Element* panel,
        SplitDirection direction,
        float ratio = 0.5f
    ) = 0;

    /// Объединить панели
    virtual bool MergePanels(Rml::Element* first, Rml::Element* second) = 0;

    /// Начать перетаскивание сплиттера
    virtual void BeginDrag(Rml::Element* splitter) = 0;

    /// Обновить позицию перетаскивания
    virtual void UpdateDrag(float mouse_x, float mouse_y) = 0;

    /// Завершить перетаскивание
    virtual void EndDrag() = 0;

    /// Сгенерировать RML для панелей
    virtual std::string GenerateRML() const = 0;

    /// Применить раскладку к контексту
    virtual void ApplyLayout() = 0;
};

} // namespace skif::rmlui