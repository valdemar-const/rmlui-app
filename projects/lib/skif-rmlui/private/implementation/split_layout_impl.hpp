#pragma once

#include <skif/rmlui/editor/i_split_layout.hpp>
#include <skif/rmlui/editor/i_editor_host.hpp>
#include <skif/rmlui/editor/i_editor_registry.hpp>

#include <memory>
#include <set>
#include <string>
#include <vector>
#include <cstdint>

// Forward declaration RmlUi
namespace Rml
{
class Context;
class Element;
class ElementDocument;
class Event;
} // namespace Rml

namespace skif::rmlui
{

class PanelContainerController;

/**
 * @brief Реализация рекурсивного дерева панелей
 * @note Обходит дерево SplitNode рекурсивно для обновления редакторов.
 *       Генерирует единый RML документ с panel containers.
 *       Загружает Editor RML контент в panel-content div.
 *       Привязывает event listeners через PanelContainerController.
 */
class SplitLayoutImpl final : public ISplitLayout
{
public:
    SplitLayoutImpl();
    ~SplitLayoutImpl() override;

    // ISplitLayout
    void SetContext(Rml::Context* context) override;
    void SetEditorHost(IEditorHost* host) override;
    void SetEditorRegistry(IEditorRegistry* registry) override;
    void SetRoot(std::unique_ptr<SplitNode> root) override;
    [[nodiscard]] const SplitNode* GetRoot() const noexcept override;
    bool Split(const SplitNode* panel, SplitDirection direction,
               std::string_view new_editor_name, float ratio) override;
    bool Merge(const SplitNode* split_node, bool keep_first) override;
    bool SwitchEditor(const SplitNode* panel, std::string_view new_editor_name) override;
    void Update(float delta_time) override;
    void Initialize() override;
    void ApplyLayout() override;
    [[nodiscard]] std::string GenerateRML() const override;

    /// Получить layout документ (для EditorHost)
    [[nodiscard]] Rml::ElementDocument* GetLayoutDocument() const noexcept { return layout_document_; }

    /// Найти SplitNode по instance_id
    [[nodiscard]] SplitNode* FindNodeByInstanceId(std::string_view instance_id);

private:
    /// Рекурсивно обновить редакторы в поддереве
    void UpdateRecursive(const SplitNode* node, float delta_time);

    /// Рекурсивно инициализировать редакторы (создать экземпляры)
    void InitializeRecursive(const SplitNode* node);

    /// Рекурсивно уничтожить редакторы
    void DestroyRecursive(const SplitNode* node);

    /// Сгенерировать RML для узла рекурсивно
    void GenerateRMLRecursive(const SplitNode* node, std::string& output, int depth) const;

    /// Сгенерировать RML для panel container (leaf-узел)
    void GeneratePanelContainerRML(const SplitNode* node, std::string& output, int depth) const;

    /// Сгенерировать editor switcher dropdown
    void GenerateEditorSwitcherRML(const SplitNode* node, std::string& output, int depth) const;

    /// Загрузить Editor RML контент в panel-content div для всех leaf-узлов
    void LoadEditorContentRecursive(const SplitNode* node);

    /// Прочитать файл и извлечь содержимое <body>...</body>
    [[nodiscard]] std::string ReadRmlBodyContent(const std::string& rml_path) const;

    /// Прочитать содержимое <style>...</style> из RML файла
    [[nodiscard]] std::string ReadRmlStyles(const std::string& rml_path) const;

    /// Создать PanelContainerController для всех leaf-узлов
    void SetupPanelControllersRecursive(const SplitNode* node);

    /// Привязать drag-обработчики к dividers
    void SetupDividerHandlers();

    /// Обработчики divider drag
    void OnDividerMouseDown(Rml::Event& event, Rml::Element* divider);
    void OnDocumentMouseMove(Rml::Event& event);
    void OnDocumentMouseUp(Rml::Event& event);

    /// Найти SplitNode, соответствующий divider элементу
    [[nodiscard]] SplitNode* FindSplitNodeForDivider(Rml::Element* divider);

    /// Рекурсивный поиск SplitNode по instance_id
    [[nodiscard]] SplitNode* FindNodeByInstanceIdRecursive(SplitNode* node, std::string_view instance_id);

    /// Рекурсивный поиск SplitNode по адресу (uintptr_t)
    [[nodiscard]] SplitNode* FindSplitNodeByAddress(SplitNode* node, uintptr_t address);

    /// Найти узел в дереве и его родителя
    struct FindResult
    {
        SplitNode* node   = nullptr;
        SplitNode* parent = nullptr;
        bool is_first     = false;
    };
    FindResult FindNode(SplitNode* root, const SplitNode* target);
    FindResult FindNodeRecursive(SplitNode* current, SplitNode* parent, bool is_first, const SplitNode* target);

    /// Собрать уникальные RCSS пути из дескрипторов Editor'ов в дереве
    void CollectRcssPaths(const SplitNode* node, std::set<std::string>& paths) const;

    /// Генерировать уникальный instance_id для leaf-узла
    std::string MakeInstanceId(const SplitNode* node) const;

    /// Закрыть layout документ и очистить контроллеры
    void CloseLayoutDocument();

    Rml::Context*                   context_ = nullptr;
    IEditorHost*                    editor_host_ = nullptr;
    IEditorRegistry*                editor_registry_ = nullptr;
    std::unique_ptr<SplitNode>      root_;
    uint64_t                        instance_counter_ = 0;

    /// Единый layout документ, загруженный через LoadDocumentFromMemory
    Rml::ElementDocument*           layout_document_ = nullptr;

    /// Контроллеры panel container (по одному на каждый leaf-узел)
    std::vector<std::unique_ptr<PanelContainerController>> panel_controllers_;

    /// Divider drag state
    Rml::Element*                   dragging_divider_ = nullptr;
    SplitNode*                      dragging_split_node_ = nullptr;
    float                           drag_start_mouse_ = 0.0f;
    float                           drag_start_ratio_ = 0.0f;
    float                           drag_container_size_ = 0.0f;
};

} // namespace skif::rmlui
