#pragma once

#include <skif/rmlui/editor/i_split_layout.hpp>
#include <skif/rmlui/editor/i_editor_host.hpp>

#include <memory>
#include <string>
#include <cstdint>

// Forward declaration RmlUi
namespace Rml
{
class Context;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Реализация рекурсивного дерева панелей
 * @note Обходит дерево SplitNode рекурсивно для обновления редакторов.
 */
class SplitLayoutImpl final : public ISplitLayout
{
public:
    SplitLayoutImpl();
    ~SplitLayoutImpl() override;

    // ISplitLayout
    void SetContext(Rml::Context* context) override;
    void SetEditorHost(IEditorHost* host) override;
    void SetRoot(std::unique_ptr<SplitNode> root) override;
    [[nodiscard]] const SplitNode* GetRoot() const noexcept override;
    bool Split(const SplitNode* panel, SplitDirection direction,
               std::string_view new_editor_name, float ratio) override;
    bool Merge(const SplitNode* split_node, bool keep_first) override;
    void Update(float delta_time) override;
    void Initialize() override;
    void ApplyLayout() override;
    [[nodiscard]] std::string GenerateRML() const override;

private:
    /// Рекурсивно обновить редакторы в поддереве
    void UpdateRecursive(const SplitNode* node, float delta_time);

    /// Рекурсивно инициализировать редакторы (создать экземпляры)
    void InitializeRecursive(const SplitNode* node);

    /// Рекурсивно уничтожить редакторы
    void DestroyRecursive(const SplitNode* node);

    /// Сгенерировать RML для узла рекурсивно
    void GenerateRMLRecursive(const SplitNode* node, std::string& output, int depth) const;

    /// Найти узел в дереве и его родителя
    struct FindResult
    {
        SplitNode* node   = nullptr;
        SplitNode* parent = nullptr;
        bool is_first     = false;  // true если node == parent->first
    };
    FindResult FindNode(SplitNode* root, const SplitNode* target);
    FindResult FindNodeRecursive(SplitNode* current, SplitNode* parent, bool is_first, const SplitNode* target);

    /// Генерировать уникальный instance_id для leaf-узла
    std::string MakeInstanceId(const SplitNode* node) const;

    Rml::Context*                   context_ = nullptr;
    IEditorHost*                    editor_host_ = nullptr;
    std::unique_ptr<SplitNode>      root_;
    uint64_t                        instance_counter_ = 0;
};

} // namespace skif::rmlui
