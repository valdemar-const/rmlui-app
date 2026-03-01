#pragma once

#include <skif/rmlui/plugin/i_plugin.hpp>
#include <skif/rmlui/editor/i_editor.hpp>

#include <memory>
#include <string_view>

namespace sample
{

/**
 * @brief Sample Editor — демонстрационный редактор с кнопкой и счётчиком
 */
class SampleEditor : public skif::rmlui::IEditor
{
public:
    SampleEditor();
    ~SampleEditor() override;

    [[nodiscard]] const skif::rmlui::EditorDescriptor& GetDescriptor() const noexcept override;

    void OnCreated(Rml::ElementDocument* document) override;
    void OnActivate() override;
    void OnDeactivate() override;
    void OnUpdate(float delta_time) override;
    void OnDispose() noexcept override;

private:
    skif::rmlui::EditorDescriptor descriptor_;
    Rml::ElementDocument* document_ = nullptr;
    int counter_ = 0;
    
    void UpdateCounterDisplay();
};

/**
 * @brief Sample Plugin — демонстрационный плагин, регистрирующий SampleEditor
 */
class SamplePanelPlugin : public skif::rmlui::IPlugin
{
public:
    SamplePanelPlugin();

    [[nodiscard]] std::string_view GetName() const noexcept override;
    [[nodiscard]] skif::rmlui::Version GetVersion() const noexcept override;
    [[nodiscard]] std::string_view GetDescription() const noexcept override;

    void OnLoad(skif::rmlui::IPluginRegistry& registry) override;
    void OnUnload() noexcept override;
};

} // namespace sample
