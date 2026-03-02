#pragma once

#include <skif/rmlui/editor/i_editor.hpp>

#include <memory>
#include <string_view>

// Forward declaration RmlUi
namespace Rml
{
class Context;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Интерфейс хоста редакторов
 * @note Управляет жизненным циклом экземпляров Editor.
 *       Создаёт редакторы из IEditorRegistry, загружает RML документы,
 *       управляет активацией/деактивацией.
 */
class IEditorHost
{
  public:

    virtual ~IEditorHost() = default;

    /// Установить RmlUi контекст
    virtual void SetContext(Rml::Context *context) = 0;

    /// Создать и активировать экземпляр редактора по имени
    /// @param editor_name Имя зарегистрированного типа редактора
    /// @param instance_id Уникальный идентификатор экземпляра (для нескольких панелей с одним типом)
    /// @return true если редактор успешно создан
    [[nodiscard]] virtual bool CreateEditor(std::string_view editor_name, std::string_view instance_id) = 0;

    /// Уничтожить экземпляр редактора
    virtual void DestroyEditor(std::string_view instance_id) = 0;

    /// Активировать редактор (сделать видимым)
    virtual void ActivateEditor(std::string_view instance_id) = 0;

    /// Деактивировать редактор (скрыть)
    virtual void DeactivateEditor(std::string_view instance_id) = 0;

    /// Получить экземпляр редактора по instance_id
    [[nodiscard]] virtual IEditor *GetEditor(std::string_view instance_id) const = 0;

    /// Обновить конкретный редактор
    virtual void UpdateEditor(std::string_view instance_id, float delta_time) = 0;

    /// Обновить все активные редакторы
    virtual void UpdateAll(float delta_time) = 0;

    /// Уничтожить все редакторы
    virtual void DestroyAll() = 0;
};

} // namespace skif::rmlui
