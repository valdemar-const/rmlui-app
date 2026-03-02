#pragma once

#include <skif/rmlui/view/i_view.hpp>

#include <memory>
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
 * @brief Интерфейс хоста представлений
 * @note Управляет отображением view в RmlUi контексте
 */
class IViewHost
{
  public:

    virtual ~IViewHost() = default;

    /// Установить RmlUi контекст
    virtual void SetContext(Rml::Context *context) = 0;

    /// Присоединить представление к контейнеру
    virtual bool AttachView(std::string_view view_name, Rml::Element *container) = 0;

    /// Отсоединить представление от контейнера
    virtual void DetachView(Rml::Element *container) = 0;

    /// Получить активное представление
    [[nodiscard]] virtual IView *GetActiveView() const = 0;

    /// Показать представление по имени
    virtual void ShowView(std::string_view name) = 0;

    /// Скрыть представление по имени
    virtual void HideView(std::string_view name) = 0;

    /// Обновить все представления
    virtual void Update(float delta_time) = 0;
};

} // namespace skif::rmlui
