#pragma once

#include <skif/rmlui/editor/i_editor_registry.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace skif::rmlui
{

/**
 * @brief Реализация реестра редакторов
 */
class EditorRegistryImpl final : public IEditorRegistry
{
  public:

    EditorRegistryImpl()           = default;
    ~EditorRegistryImpl() override = default;

    // IEditorRegistry
    void                                                RegisterEditor(EditorDescriptor descriptor, EditorFactory factory) override;
    [[nodiscard]] std::unique_ptr<IEditor>              CreateEditor(std::string_view name) const override;
    [[nodiscard]] const EditorDescriptor               *GetDescriptor(std::string_view name) const override;
    [[nodiscard]] std::vector<const EditorDescriptor *> GetAllDescriptors() const override;
    [[nodiscard]] std::vector<const EditorDescriptor *> GetDescriptorsByCategory(
            std::string_view category
    ) const override;
    [[nodiscard]] std::vector<std::string> GetCategories() const override;

  private:

    struct EditorEntry
    {
        EditorDescriptor descriptor;
        EditorFactory    factory;
    };

    std::unordered_map<std::string, EditorEntry> editors_;
};

} // namespace skif::rmlui
