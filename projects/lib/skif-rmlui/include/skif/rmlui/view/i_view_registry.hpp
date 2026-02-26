#pragma once

#include <skif/rmlui/view/i_view.hpp>
#include <skif/rmlui/view/view_descriptor.hpp>

#include <memory>
#include <string_view>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Интерфейс реестра представлений
 * @note Используется для регистрации и создания view плагинами
 */
class IViewRegistry
{
public:
    virtual ~IViewRegistry() = default;

    /// Зарегистрировать представление
    virtual void RegisterView(ViewDescriptor descriptor, ViewFactory factory) = 0;

    /// Создать представление по имени
    [[nodiscard]] virtual std::unique_ptr<IView> CreateView(std::string_view name) const = 0;

    /// Получить дескриптор представления
    [[nodiscard]] virtual const ViewDescriptor* GetDescriptor(std::string_view name) const = 0;

    /// Получить все дескрипторы
    [[nodiscard]] virtual std::vector<const ViewDescriptor*> GetAllDescriptors() const = 0;

    /// Получить дескрипторы по категории
    [[nodiscard]] virtual std::vector<const ViewDescriptor*> GetDescriptorsByCategory(
        std::string_view category
    ) const = 0;

    /// Получить список категорий
    [[nodiscard]] virtual std::vector<std::string> GetCategories() const = 0;
};

} // namespace skif::rmlui