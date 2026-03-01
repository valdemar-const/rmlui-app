#include <implementation/editor_host_impl.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Log.h>

#include <fstream>
#include <sstream>

namespace skif::rmlui
{

EditorHostImpl::EditorHostImpl(IEditorRegistry& registry)
    : registry_(registry)
{}

EditorHostImpl::~EditorHostImpl()
{
    DestroyAll();
}

void
EditorHostImpl::SetContext(Rml::Context* context)
{
    context_ = context;
}

bool
EditorHostImpl::CreateEditor(std::string_view editor_name, std::string_view instance_id)
{
    if (!context_)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "EditorHost: No RmlUi context set.");
        return false;
    }

    const std::string id_str(instance_id);

    // Проверяем, не существует ли уже экземпляр
    if (instances_.contains(id_str))
    {
        Rml::Log::Message(Rml::Log::LT_WARNING,
            "EditorHost: Instance '%s' already exists.", instance_id.data());
        return false;
    }

    // Получаем дескриптор
    const EditorDescriptor* descriptor = registry_.GetDescriptor(editor_name);
    if (!descriptor)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Editor type '%s' not found in registry.", editor_name.data());
        return false;
    }

    // Создаём экземпляр редактора
    auto editor = registry_.CreateEditor(editor_name);
    if (!editor)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Failed to create editor '%s'.", editor_name.data());
        return false;
    }

    // Загружаем RML документ
    Rml::ElementDocument* document = context_->LoadDocument(descriptor->rml_path.c_str());
    if (!document)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Failed to load RML '%s' for editor '%s'.",
            descriptor->rml_path.c_str(), editor_name.data());
        return false;
    }

    // Уведомляем редактор о создании
    editor->OnCreated(document);

    // Сохраняем экземпляр
    EditorInstance instance;
    instance.editor   = std::move(editor);
    instance.document = document;
    instance.active   = false;
    instance.embedded = false;

    instances_[id_str] = std::move(instance);

    Rml::Log::Message(Rml::Log::LT_INFO,
        "EditorHost: Created editor '%s' (instance '%s').",
        editor_name.data(), instance_id.data());

    return true;
}

bool
EditorHostImpl::CreateEditorEmbedded(
    std::string_view editor_name,
    std::string_view instance_id,
    Rml::Element* content_element,
    Rml::ElementDocument* layout_document)
{
    if (!context_ || !content_element || !layout_document)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Invalid parameters for CreateEditorEmbedded.");
        return false;
    }

    const std::string id_str(instance_id);

    // Проверяем, не существует ли уже экземпляр
    if (instances_.contains(id_str))
    {
        Rml::Log::Message(Rml::Log::LT_WARNING,
            "EditorHost: Instance '%s' already exists.", instance_id.data());
        return false;
    }

    // Получаем дескриптор
    const EditorDescriptor* descriptor = registry_.GetDescriptor(editor_name);
    if (!descriptor)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Editor type '%s' not found in registry.", editor_name.data());
        return false;
    }

    // Создаём экземпляр редактора
    auto editor = registry_.CreateEditor(editor_name);
    if (!editor)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Failed to create editor '%s'.", editor_name.data());
        return false;
    }

    // Читаем RML файл и извлекаем body-контент
    std::string rml_content = ReadFile(descriptor->rml_path);
    if (rml_content.empty())
    {
        Rml::Log::Message(Rml::Log::LT_ERROR,
            "EditorHost: Failed to read RML file '%s'.", descriptor->rml_path.c_str());
        return false;
    }

    // Извлекаем body-контент
    std::string body_content = ExtractBodyContent(rml_content);
    if (body_content.empty())
    {
        Rml::Log::Message(Rml::Log::LT_WARNING,
            "EditorHost: Empty body in RML '%s'.", descriptor->rml_path.c_str());
    }

    // Стили Editor'а подключаются через <link> в layout RML (rcss_path в EditorDescriptor)
    // Inline стили из <style> тега Editor RML не переносятся — используйте RCSS файл

    // Вставляем body-контент в panel-content элемент
    content_element->SetInnerRML(body_content);

    // Уведомляем редактор о создании — передаём layout документ и content_element
    // Editor может использовать content_element для scoped поиска элементов
    editor->OnCreatedInContainer(layout_document, content_element);

    // Сохраняем экземпляр
    EditorInstance instance;
    instance.editor   = std::move(editor);
    instance.document = layout_document;
    instance.active   = false;
    instance.embedded = true;

    instances_[id_str] = std::move(instance);

    Rml::Log::Message(Rml::Log::LT_INFO,
        "EditorHost: Created embedded editor '%s' (instance '%s').",
        editor_name.data(), instance_id.data());

    return true;
}

void
EditorHostImpl::DestroyEditor(std::string_view instance_id)
{
    const std::string id_str(instance_id);
    auto it = instances_.find(id_str);
    if (it == instances_.end())
    {
        return;
    }

    auto& inst = it->second;

    // Деактивируем если активен
    if (inst.active)
    {
        inst.editor->OnDeactivate();
    }

    // Уничтожаем
    inst.editor->OnDispose();

    // Закрываем документ только для legacy (не embedded) режима
    if (inst.document && !inst.embedded)
    {
        inst.document->Close();
    }

    instances_.erase(it);
}

void
EditorHostImpl::ActivateEditor(std::string_view instance_id)
{
    const std::string id_str(instance_id);
    auto it = instances_.find(id_str);
    if (it == instances_.end())
    {
        return;
    }

    auto& inst = it->second;
    if (!inst.active)
    {
        if (inst.document && !inst.embedded)
        {
            inst.document->Show();
        }
        inst.editor->OnActivate();
        inst.active = true;
    }
}

void
EditorHostImpl::DeactivateEditor(std::string_view instance_id)
{
    const std::string id_str(instance_id);
    auto it = instances_.find(id_str);
    if (it == instances_.end())
    {
        return;
    }

    auto& inst = it->second;
    if (inst.active)
    {
        inst.editor->OnDeactivate();
        if (inst.document && !inst.embedded)
        {
            inst.document->Hide();
        }
        inst.active = false;
    }
}

IEditor*
EditorHostImpl::GetEditor(std::string_view instance_id) const
{
    const std::string id_str(instance_id);
    auto it = instances_.find(id_str);
    if (it != instances_.end())
    {
        return it->second.editor.get();
    }
    return nullptr;
}

void
EditorHostImpl::UpdateEditor(std::string_view instance_id, float delta_time)
{
    const std::string id_str(instance_id);
    auto it = instances_.find(id_str);
    if (it != instances_.end() && it->second.active)
    {
        it->second.editor->OnUpdate(delta_time);
    }
}

void
EditorHostImpl::UpdateAll(float delta_time)
{
    for (auto& [id, inst] : instances_)
    {
        if (inst.active)
        {
            inst.editor->OnUpdate(delta_time);
        }
    }
}

void
EditorHostImpl::DestroyAll()
{
    // Деактивируем и уничтожаем все экземпляры
    for (auto& [id, inst] : instances_)
    {
        if (inst.active)
        {
            inst.editor->OnDeactivate();
            inst.active = false;
        }
        inst.editor->OnDispose();
        if (inst.document && !inst.embedded)
        {
            inst.document->Close();
        }
    }
    instances_.clear();
}

// ============================================================================
// Private helpers
// ============================================================================

std::string
EditorHostImpl::ReadFile(const std::string& path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        return {};
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string
EditorHostImpl::ExtractBodyContent(const std::string& rml_content) const
{
    // Ищем <body> и </body> теги
    auto body_start = rml_content.find("<body");
    if (body_start == std::string::npos)
    {
        return {};
    }

    // Пропускаем атрибуты тега <body ...>
    auto body_content_start = rml_content.find('>', body_start);
    if (body_content_start == std::string::npos)
    {
        return {};
    }
    body_content_start++; // Пропускаем '>'

    auto body_end = rml_content.find("</body>", body_content_start);
    if (body_end == std::string::npos)
    {
        return {};
    }

    return rml_content.substr(body_content_start, body_end - body_content_start);
}

std::string
EditorHostImpl::ExtractStyleContent(const std::string& rml_content) const
{
    // Ищем <style> и </style> теги
    auto style_start = rml_content.find("<style");
    if (style_start == std::string::npos)
    {
        return {};
    }

    auto style_content_start = rml_content.find('>', style_start);
    if (style_content_start == std::string::npos)
    {
        return {};
    }
    style_content_start++; // Пропускаем '>'

    auto style_end = rml_content.find("</style>", style_content_start);
    if (style_end == std::string::npos)
    {
        return {};
    }

    return rml_content.substr(style_content_start, style_end - style_content_start);
}

} // namespace skif::rmlui
