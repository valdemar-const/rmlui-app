#pragma once

#include <skif/rmlui/editor/i_editor_host.hpp>
#include <skif/rmlui/editor/i_editor_registry.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// Forward declaration RmlUi
namespace Rml
{
class Context;
class ElementDocument;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Реализация хоста редакторов
 * @note Управляет жизненным циклом экземпляров Editor.
 */
class EditorHostImpl final : public IEditorHost
{
public:
    explicit EditorHostImpl(IEditorRegistry& registry);
    ~EditorHostImpl() override;

    // IEditorHost
    void SetContext(Rml::Context* context) override;
    [[nodiscard]] bool CreateEditor(std::string_view editor_name, std::string_view instance_id) override;
    void DestroyEditor(std::string_view instance_id) override;
    void ActivateEditor(std::string_view instance_id) override;
    void DeactivateEditor(std::string_view instance_id) override;
    [[nodiscard]] IEditor* GetEditor(std::string_view instance_id) const override;
    void UpdateEditor(std::string_view instance_id, float delta_time) override;
    void UpdateAll(float delta_time) override;
    void DestroyAll() override;

private:
    struct EditorInstance
    {
        std::unique_ptr<IEditor>  editor;
        Rml::ElementDocument*     document = nullptr;
        bool                      active   = false;
    };

    IEditorRegistry&                                    registry_;
    Rml::Context*                                       context_ = nullptr;
    std::unordered_map<std::string, EditorInstance>     instances_;
};

} // namespace skif::rmlui
