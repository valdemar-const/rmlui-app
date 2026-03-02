#pragma once

#include <skif/rmlui/editor/i_editor.hpp>
#include <skif/rmlui/editor/editor_descriptor.hpp>

#include <memory>
#include <string_view>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Интерфейс реестра редакторов — contribution point
 * @note Плагины регистрируют свои типы редакторов через этот интерфейс.
 *       EditorHost использует его для создания экземпляров редакторов.
 */
class IEditorRegistry
{
  public:

    virtual ~IEditorRegistry() = default;

    /// Зарегистрировать тип редактора (вызывается из плагина)
    virtual void RegisterEditor(EditorDescriptor descriptor, EditorFactory factory) = 0;

    /// Создать экземпляр редактора по имени
    [[nodiscard]] virtual std::unique_ptr<IEditor> CreateEditor(std::string_view name) const = 0;

    /// Получить дескриптор по имени
    [[nodiscard]] virtual const EditorDescriptor *GetDescriptor(std::string_view name) const = 0;

    /// Получить все зарегистрированные дескрипторы
    [[nodiscard]] virtual std::vector<const EditorDescriptor *> GetAllDescriptors() const = 0;

    /// Получить дескрипторы по категории
    [[nodiscard]] virtual std::vector<const EditorDescriptor *> GetDescriptorsByCategory(
            std::string_view category
    ) const = 0;

    /// Получить список категорий
    [[nodiscard]] virtual std::vector<std::string> GetCategories() const = 0;
};

} // namespace skif::rmlui
