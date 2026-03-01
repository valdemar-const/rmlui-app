#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/editor/editor_descriptor.hpp>

#include <functional>
#include <memory>
#include <string_view>

// Forward declaration RmlUi
namespace Rml
{
class Element;
class ElementDocument;
class Event;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Интерфейс редактора (Editor)
 * @note Каждый Editor — самодостаточная единица UI в панели SplitLayout.
 *       Имеет собственный RML документ, меню и жизненный цикл.
 *       Логика на C++, RML — только presentation layer.
 *
 * Жизненный цикл:
 *   [Created] → OnCreated(document)
 *   [Active]  → OnActivate() / OnUpdate(dt)
 *   [Inactive]→ OnDeactivate()
 *   [Disposed]→ OnDispose()
 */
class IEditor
{
public:
    virtual ~IEditor() = default;

    /// Получить дескриптор редактора
    [[nodiscard]] virtual const EditorDescriptor& GetDescriptor() const noexcept = 0;

    /// Вызывается при создании — RML документ загружен и привязан
    virtual void OnCreated(Rml::ElementDocument* document) = 0;

    /// Вызывается при активации — редактор стал видимым в панели
    virtual void OnActivate() = 0;

    /// Вызывается при деактивации — редактор скрыт (панель свёрнута или заменена)
    virtual void OnDeactivate() = 0;

    /// Вызывается каждый кадр для активных редакторов
    virtual void OnUpdate(float delta_time) = 0;

    /// Вызывается при уничтожении — освободить ресурсы
    virtual void OnDispose() noexcept = 0;

    /// Получить текст статус бара (опционально, по умолчанию пустой)
    [[nodiscard]] virtual std::string_view GetStatusText() const noexcept
    {
        return {};
    }
};

/**
 * @brief Фабрика редакторов
 */
using EditorFactory = std::function<std::unique_ptr<IEditor>()>;

} // namespace skif::rmlui
