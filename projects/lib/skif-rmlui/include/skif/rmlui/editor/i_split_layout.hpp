#pragma once

#include <skif/rmlui/editor/split_node.hpp>

#include <memory>
#include <string_view>

// Forward declaration RmlUi
namespace Rml
{
class Context;
} // namespace Rml

namespace skif::rmlui
{

class IEditorHost;

/**
 * @brief Интерфейс рекурсивного дерева панелей (SplitLayout)
 * @note Аналог системы Areas в Blender.
 *       Дерево SplitNode определяет расположение панелей.
 *       Каждый leaf-узел содержит экземпляр Editor.
 *       Обновление идёт рекурсивно от корня вниз.
 */
class ISplitLayout
{
public:
    virtual ~ISplitLayout() = default;

    /// Установить RmlUi контекст
    virtual void SetContext(Rml::Context* context) = 0;

    /// Установить EditorHost для создания/управления редакторами
    virtual void SetEditorHost(IEditorHost* host) = 0;

    /// Установить корневой узел дерева
    virtual void SetRoot(std::unique_ptr<SplitNode> root) = 0;

    /// Получить корневой узел
    [[nodiscard]] virtual const SplitNode* GetRoot() const noexcept = 0;

    /// Разделить панель (leaf-узел) на две
    /// @param panel Leaf-узел для разделения
    /// @param direction Направление разделения
    /// @param new_editor_name Имя редактора для новой панели
    /// @param ratio Пропорция разделения
    virtual bool Split(
        const SplitNode* panel,
        SplitDirection direction,
        std::string_view new_editor_name,
        float ratio = 0.5f
    ) = 0;

    /// Объединить split-узел (удалить разделение, оставить один из дочерних)
    /// @param split_node Split-узел для объединения
    /// @param keep_first true — оставить первый дочерний, false — второй
    virtual bool Merge(const SplitNode* split_node, bool keep_first = true) = 0;

    /// Рекурсивно обновить все активные редакторы в дереве
    virtual void Update(float delta_time) = 0;

    /// Инициализировать layout — создать редакторы для всех leaf-узлов
    virtual void Initialize() = 0;

    /// Применить layout к RmlUi контексту (сгенерировать RML)
    virtual void ApplyLayout() = 0;

    /// Сгенерировать RML разметку для текущего дерева
    [[nodiscard]] virtual std::string GenerateRML() const = 0;
};

} // namespace skif::rmlui
