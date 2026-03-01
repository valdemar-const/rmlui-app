#pragma once

#include <skif/rmlui/plugin/i_plugin.hpp>
#include <skif/rmlui/view/i_view.hpp>

#include <memory>
#include <string_view>

namespace sample
{

/**
 * @brief Sample View - демонстрационный View с кнопкой и счётчиком
 */
class SamplePanelView : public skif::rmlui::IView
{
public:
    SamplePanelView();
    ~SamplePanelView() override;

    [[nodiscard]] const skif::rmlui::ViewDescriptor& GetDescriptor() const noexcept override;

    void OnCreated(Rml::ElementDocument* document) override;
    void OnDestroyed() noexcept override;
    void OnShow() override;
    void OnHide() override;
    void OnUpdate(float delta_time) override;

private:
    skif::rmlui::ViewDescriptor descriptor_;
    Rml::ElementDocument* document_ = nullptr;
    int counter_ = 0;
    
    void UpdateCounterDisplay();
};

/**
 * @brief Sample Plugin - демонстрационный плагин
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

private:
    std::unique_ptr<SamplePanelView> view_;
};

} // namespace sample
