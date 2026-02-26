#pragma once

#include <skif/rmlui/view/i_view_registry.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Реализация реестра представлений
 */
class ViewRegistryImpl final : public IViewRegistry
{
public:
    ViewRegistryImpl() = default;
    ~ViewRegistryImpl() override = default;

    // IViewRegistry
    void RegisterView(ViewDescriptor descriptor, ViewFactory factory) override;
    [[nodiscard]] std::unique_ptr<IView> CreateView(std::string_view name) const override;
    [[nodiscard]] const ViewDescriptor* GetDescriptor(std::string_view name) const override;
    [[nodiscard]] std::vector<const ViewDescriptor*> GetAllDescriptors() const override;
    [[nodiscard]] std::vector<const ViewDescriptor*> GetDescriptorsByCategory(
        std::string_view category
    ) const override;
    [[nodiscard]] std::vector<std::string> GetCategories() const override;

private:
    struct ViewEntry
    {
        ViewDescriptor descriptor;
        ViewFactory    factory;
    };

    std::unordered_map<std::string, ViewEntry> views_;
};

} // namespace skif::rmlui