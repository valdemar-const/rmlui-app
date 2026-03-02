#pragma once

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/EventListener.h>
#include <functional>

namespace skif::rmlui
{

/// Обёртка для std::function в RmlUi EventListener API
/// Автоматически удаляется при отсоединении от элемента
class LambdaEventListener final : public Rml::EventListener
{
  public:

    using Callback = std::function<void(Rml::Event &)>;

    explicit LambdaEventListener(Callback callback)
        : callback_(std::move(callback))
    {
    }

    void
    ProcessEvent(Rml::Event &event) override
    {
        if (callback_)
        {
            callback_(event);
        }
    }

    void
    OnDetach(Rml::Element * /*element*/) override
    {
        delete this;
    }

  private:

    Callback callback_;
};

/// Удобная функция для привязки обработчика события к элементу
/// @param element Элемент, к которому привязывается обработчик
/// @param event_name Имя события (например, "click", "mouseover")
/// @param handler Функция-обработчик
inline void
BindEvent(
        Rml::Element                     *element,
        const Rml::String                &event_name,
        std::function<void(Rml::Event &)> handler
)
{
    if (element)
    {
        element->AddEventListener(event_name, new LambdaEventListener(std::move(handler)));
    }
}

} // namespace skif::rmlui
