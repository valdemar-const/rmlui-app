#include "sample_panel.hpp"

#include <skif/rmlui/plugin/i_plugin_registry.hpp>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>

#include <string>
#include <memory>

namespace sample
{

// ============================================================================
// LambdaEventListener - обёртка для std::function в RmlUi EventListener
// ============================================================================

class LambdaEventListener : public Rml::EventListener
{
public:
    using Callback = std::function<void(Rml::Event&)>;
    
    explicit LambdaEventListener(Callback callback)
        : callback_(std::move(callback))
    {
    }
    
    void ProcessEvent(Rml::Event& event) override
    {
        if (callback_)
        {
            callback_(event);
        }
    }
    
    void OnDetach(Rml::Element* /*element*/) override
    {
        delete this;
    }
    
private:
    Callback callback_;
};

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
    
    // Находим элементы
    auto* increment_btn = document_->GetElementById("increment-button");
    auto* reset_btn = document_->GetElementById("reset-button");
    
    // Привязываем обработчики событий через LambdaEventListener
    if (increment_btn)
    {
        increment_btn->AddEventListener("click",
            new LambdaEventListener([this](Rml::Event& event)
            {
                counter_++;
                UpdateCounterDisplay();
            })
        );
    }
    
    if (reset_btn)
    {
        reset_btn->AddEventListener("click",
            new LambdaEventListener([this](Rml::Event& event)
            {
                counter_ = 0;
                UpdateCounterDisplay();
            })
        );
    }
}

void
SamplePanelView::UpdateCounterDisplay()
{
    if (document_)
    {
        auto* counter_element = document_->GetElementById("counter-value");
        if (counter_element)
        {
            counter_element->SetInnerRML(std::to_string(counter_));
        }
    }
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
