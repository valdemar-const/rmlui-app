#include "sample_panel.hpp"

#include <skif/rmlui/plugin/i_plugin_registry.hpp>
#include <skif/rmlui/editor/i_editor_registry.hpp>
#include <skif/rmlui/view/lambda_event_listener.hpp>

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <string>
#include <memory>

namespace sample
{

// ============================================================================
// SampleEditor Implementation
// ============================================================================

SampleEditor::SampleEditor()
{
    descriptor_.name = "sample_panel";
    descriptor_.display_name = "Sample Panel";
    descriptor_.rml_path = "assets/ui/sample_panel.rml";
    descriptor_.rcss_path = "assets/ui/sample_panel.rcss";
    descriptor_.category = "Panels";
    descriptor_.menu_entries = {
        {"Increment", "sample.increment", ""},
        {"Reset", "sample.reset", "Ctrl+R"},
    };
}

SampleEditor::~SampleEditor() = default;

const skif::rmlui::EditorDescriptor&
SampleEditor::GetDescriptor() const noexcept
{
    return descriptor_;
}

void
SampleEditor::OnCreated(Rml::ElementDocument* document)
{
    document_ = document;
    // Legacy mode — поиск по всему документу
    BindEventsToContainer(document);
}

void
SampleEditor::OnCreatedInContainer(Rml::ElementDocument* document, Rml::Element* content_container)
{
    document_ = document;
    content_container_ = content_container;
    // Embedded mode — scoped поиск внутри content_container
    BindEventsToContainer(content_container);
}

void
SampleEditor::BindEventsToContainer(Rml::Element* container)
{
    if (!container)
    {
        return;
    }

    auto* increment_btn = container->QuerySelector("#increment-button");
    auto* reset_btn = container->QuerySelector("#reset-button");
    
    if (increment_btn)
    {
        skif::rmlui::BindEvent(increment_btn, "click",
            [this](Rml::Event& /*event*/)
            {
                counter_++;
                UpdateCounterDisplay();
            }
        );
    }
    
    if (reset_btn)
    {
        skif::rmlui::BindEvent(reset_btn, "click",
            [this](Rml::Event& /*event*/)
            {
                counter_ = 0;
                UpdateCounterDisplay();
            }
        );
    }
}

void
SampleEditor::UpdateCounterDisplay()
{
    // Используем scoped поиск если доступен content_container
    Rml::Element* search_root = content_container_ ? content_container_ : static_cast<Rml::Element*>(document_);
    if (search_root)
    {
        auto* counter_element = search_root->QuerySelector("#counter-value");
        if (counter_element)
        {
            counter_element->SetInnerRML(std::to_string(counter_));
        }
    }
}

void
SampleEditor::OnActivate()
{
    // Редактор стал видимым в панели
}

void
SampleEditor::OnDeactivate()
{
    // Редактор скрыт
}

void
SampleEditor::OnUpdate(float /*delta_time*/)
{
    // Обновление каждый кадр
}

void
SampleEditor::OnDispose() noexcept
{
    document_ = nullptr;
}

// ============================================================================
// SamplePanelPlugin Implementation
// ============================================================================

SamplePanelPlugin::SamplePanelPlugin() = default;

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
    return "Sample panel plugin demonstrating the Editor registration system";
}

void
SamplePanelPlugin::OnLoad(skif::rmlui::IPluginRegistry& registry)
{
    // Регистрируем Editor через contribution point
    skif::rmlui::EditorDescriptor descriptor;
    descriptor.name = "sample_panel";
    descriptor.display_name = "Sample Panel";
    descriptor.rml_path = "assets/ui/sample_panel.rml";
    descriptor.rcss_path = "assets/ui/sample_panel.rcss";
    descriptor.category = "Panels";
    descriptor.menu_entries = {
        {"Increment", "sample.increment", ""},
        {"Reset", "sample.reset", "Ctrl+R"},
    };

    registry.GetEditorRegistry().RegisterEditor(
        std::move(descriptor),
        []() { return std::make_unique<SampleEditor>(); }
    );
}

void
SamplePanelPlugin::OnUnload() noexcept
{
    // Ничего не нужно очищать — EditorHost управляет экземплярами
}

} // namespace sample
