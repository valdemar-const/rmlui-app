#pragma once

#include <skif/rmlui/config.hpp>
#include <skif/rmlui/view/view_descriptor.hpp>

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
 * @brief Интерфейс представления (view)
 * @note "Глупая" view - только presentation, логика в C++
 */
class IView
{
public:
    virtual ~IView() = default;

    /// Получить дескриптор view
    [[nodiscard]] virtual const ViewDescriptor& GetDescriptor() const noexcept = 0;

    /// Вызывается при создании документа
    virtual void OnCreated(Rml::ElementDocument* document) = 0;

    /// Вызывается при уничтожении документа
    virtual void OnDestroyed() noexcept = 0;

    /// Вызывается при показе view
    virtual void OnShow() = 0;

    /// Вызывается при скрытии view
    virtual void OnHide() = 0;

    /// Вызывается каждый кадр
    virtual void OnUpdate(float delta_time) = 0;
};

/**
 * @brief Фабрика представлений
 */
using ViewFactory = std::function<std::unique_ptr<IView>()>;

} // namespace skif::rmlui