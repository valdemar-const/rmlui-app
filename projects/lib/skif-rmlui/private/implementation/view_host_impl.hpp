#pragma once

#include <skif/rmlui/view/i_view_host.hpp>
#include <skif/rmlui/view/i_view_registry.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Forward declaration RmlUi
namespace Rml
{
class Context;
class Element;
class ElementDocument;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Реализация хоста представлений
 */
class ViewHostImpl final : public IViewHost
{
public:
    explicit ViewHostImpl(IViewRegistry& registry);
    ~ViewHostImpl() override = default;

    // IViewHost
    void SetContext(Rml::Context* context) override;
    bool AttachView(std::string_view view_name, Rml::Element* container) override;
    void DetachView(Rml::Element* container) override;
    [[nodiscard]] IView* GetActiveView() const override;
    void ShowView(std::string_view name) override;
    void HideView(std::string_view name) override;
    void Update(float delta_time) override;

private:
    struct AttachedView
    {
        std::unique_ptr<IView>    view;
        Rml::ElementDocument*     document = nullptr;
        Rml::Element*             container = nullptr;
        bool                      visible   = true;
    };

    IViewRegistry&                    registry_;
    Rml::Context*                     context_ = nullptr;
    std::unordered_map<std::string, AttachedView> attached_views_;
    std::string                       active_view_name_;
};

} // namespace skif::rmlui