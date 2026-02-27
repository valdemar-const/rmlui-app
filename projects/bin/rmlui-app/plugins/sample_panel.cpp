#include "sample_panel.hpp"

#include <skif/rmlui/plugin/i_plugin_registry.hpp>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <string>

namespace sample
{

// ============================================================================
// SamplePanelView Implementation
// ============================================================================

SamplePanelView::SamplePanelView()
{
    descriptor_.name = "sample_panel";
    descriptor_.rml_path = "assets/ui/sample_panel.rml";
    descriptor_.category = "Panels";
    descriptor_.display_name = "Sample Panel";
}

SamplePanelView::~SamplePanelView() = default;

const skif::rmlui::ViewDescriptor&
SamplePanelView::GetDescriptor() const noexcept
{
    return descriptor_;
}

void
SamplePanelView::OnCreated(Rml::ElementDocument* document)
{
    document_ = document;
    // View создана - RML документ загружен
    // Здесь можно настроить начальное состояние элементов
}

void
SamplePanelView::OnDestroyed() noexcept
{
    document_ = nullptr;
}

void
SamplePanelView::OnShow()
{
    // View стала видимой
}

void
SamplePanelView::OnHide()
{
    // View скрыта
}

void
SamplePanelView::OnUpdate(float /*delta_time*/)
{
    // Обновление каждый кадр (если нужно)
}

void
SamplePanelView::BindEvent(
    Rml::Element* /*element*/,
    std::string_view /*event_name*/,
    std::function<void(Rml::Event&)> /*handler*/)
{
    // Примечание: RmlUi использует свой собственный EventListener API
    // Для полноценной поддержки нужно создать обёртку над std::function
    // Это будет реализовано в Фазе 5: Input System
}

// ============================================================================
// SamplePanelPlugin Implementation
// ============================================================================

SamplePanelPlugin::SamplePanelPlugin()
    : view_(std::make_unique<SamplePanelView>())
{
}

std::string_view
SamplePanelPlugin::GetName() const noexcept
{
    return "sample_panel";
}

skif::rmlui::Version
SamplePanelPlugin::GetVersion() const noexcept
{
    return {1, 0, 0};
}

std::string_view
SamplePanelPlugin::GetDescription() const noexcept
{
    return "Sample panel plugin demonstrating the View registration system";
}

void
SamplePanelPlugin::OnLoad(skif::rmlui::IPluginRegistry& registry)
{
    // Регистрируем View в реестре
    registry.GetViewRegistry().RegisterView(
        view_->GetDescriptor(),
        [this]() { return std::make_unique<SamplePanelView>(); }
    );
}

void
SamplePanelPlugin::OnUnload() noexcept
{
    view_.reset();
}

} // namespace sample
