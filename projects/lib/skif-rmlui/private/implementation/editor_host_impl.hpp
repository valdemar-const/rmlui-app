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
class Element;
class ElementDocument;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Реализация хоста редакторов
 * @note Управляет жизненным циклом экземпляров Editor.
 *       Поддерживает два режима создания:
 *       1. Legacy: загрузка RML как отдельного документа
 *       2. Embedded: вставка Editor RML контента в panel-content элемент layout документа
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

    /// Создать редактор в embedded режиме — контент вставляется в panel-content элемент
    /// @param editor_name Имя зарегистрированного типа редактора
    /// @param instance_id Уникальный идентификатор экземпляра
    /// @param content_element Элемент panel-content, куда вставить RML контент
    /// @param layout_document Layout документ (для OnCreated)
    /// @return true если редактор успешно создан
    [[nodiscard]] bool CreateEditorEmbedded(
        std::string_view editor_name,
        std::string_view instance_id,
        Rml::Element* content_element,
        Rml::ElementDocument* layout_document
    );

    /// Переименовать instance_id существующего editor
    /// @return true если переименование успешно
    bool RenameInstance(std::string_view old_id, std::string_view new_id);

    /// Перепривязать существующий editor к новому layout документу
    /// Используется при пересоздании layout (Split/Merge) для сохранения состояния editor
    /// @param instance_id Уникальный идентификатор экземпляра
    /// @param content_element Новый элемент panel-content
    /// @param layout_document Новый layout документ
    /// @return true если editor успешно перепривязан
    [[nodiscard]] bool ReattachEditorEmbedded(
        std::string_view instance_id,
        Rml::Element* content_element,
        Rml::ElementDocument* layout_document
    );

private:
    /// Прочитать файл и вернуть содержимое
    [[nodiscard]] std::string ReadFile(const std::string& path) const;

    /// Извлечь содержимое <body>...</body> из RML строки
    [[nodiscard]] std::string ExtractBodyContent(const std::string& rml_content) const;

    /// Извлечь содержимое <style>...</style> из RML строки
    [[nodiscard]] std::string ExtractStyleContent(const std::string& rml_content) const;

    struct EditorInstance
    {
        std::unique_ptr<IEditor>  editor;
        Rml::ElementDocument*     document = nullptr;
        bool                      active   = false;
        bool                      embedded = false;  // true если создан через CreateEditorEmbedded
    };

    IEditorRegistry&                                    registry_;
    Rml::Context*                                       context_ = nullptr;
    std::unordered_map<std::string, EditorInstance>     instances_;
};

} // namespace skif::rmlui
