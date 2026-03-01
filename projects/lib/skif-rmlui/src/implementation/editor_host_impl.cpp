#include <implementation/editor_host_impl.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Log.h>

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

    instances_[id_str] = std::move(instance);

    Rml::Log::Message(Rml::Log::LT_INFO,
        "EditorHost: Created editor '%s' (instance '%s').",
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

    // Закрываем документ
    if (inst.document)
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
        if (inst.document)
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
        if (inst.document)
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
        if (inst.document)
        {
            inst.document->Close();
        }
    }
    instances_.clear();
}

} // namespace skif::rmlui
