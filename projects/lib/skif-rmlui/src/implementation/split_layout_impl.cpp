#include <implementation/split_layout_impl.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Log.h>

#include <cassert>

namespace skif::rmlui
{

SplitLayoutImpl::SplitLayoutImpl() = default;

SplitLayoutImpl::~SplitLayoutImpl()
{
    if (root_ && editor_host_)
    {
        DestroyRecursive(root_.get());
    }
}

void
SplitLayoutImpl::SetContext(Rml::Context* context)
{
    context_ = context;
}

void
SplitLayoutImpl::SetEditorHost(IEditorHost* host)
{
    editor_host_ = host;
}

void
SplitLayoutImpl::SetRoot(std::unique_ptr<SplitNode> root)
{
    // Уничтожаем старые редакторы
    if (root_ && editor_host_)
    {
        DestroyRecursive(root_.get());
    }

    root_ = std::move(root);
}

const SplitNode*
SplitLayoutImpl::GetRoot() const noexcept
{
    return root_.get();
}

bool
SplitLayoutImpl::Split(
    const SplitNode* panel,
    SplitDirection direction,
    std::string_view new_editor_name,
    float ratio)
{
    if (!panel || !root_ || !panel->IsLeaf())
    {
        return false;
    }

    // Найти узел в дереве
    auto result = FindNode(root_.get(), panel);
    if (!result.node)
    {
        return false;
    }

    // Сохраняем данные текущего leaf
    std::string old_editor_name = result.node->editor_name;

    // Превращаем leaf в split
    result.node->editor_name.clear();
    result.node->direction = direction;
    result.node->ratio = ratio;
    result.node->first = SplitNode::MakeLeaf(old_editor_name);
    result.node->second = SplitNode::MakeLeaf(new_editor_name);

    // Создаём редактор для новой панели
    if (editor_host_)
    {
        std::string new_id = MakeInstanceId(result.node->second.get());
        editor_host_->CreateEditor(new_editor_name, new_id);
        editor_host_->ActivateEditor(new_id);
    }

    ApplyLayout();
    return true;
}

bool
SplitLayoutImpl::Merge(const SplitNode* split_node, bool keep_first)
{
    if (!split_node || !root_ || split_node->IsLeaf())
    {
        return false;
    }

    auto result = FindNode(root_.get(), split_node);
    if (!result.node || !result.node->IsSplit())
    {
        return false;
    }

    // Определяем какой дочерний узел оставить, какой удалить
    SplitNode* keep_node = keep_first ? result.node->first.get() : result.node->second.get();
    SplitNode* remove_node = keep_first ? result.node->second.get() : result.node->first.get();

    // Уничтожаем редакторы удаляемого поддерева
    if (editor_host_)
    {
        DestroyRecursive(remove_node);
    }

    // Копируем данные оставляемого узла в текущий
    if (keep_node->IsLeaf())
    {
        result.node->editor_name = keep_node->editor_name;
        result.node->first.reset();
        result.node->second.reset();
    }
    else
    {
        result.node->direction = keep_node->direction;
        result.node->ratio = keep_node->ratio;
        // Перемещаем дочерние узлы
        auto first = std::move(keep_node->first);
        auto second = std::move(keep_node->second);
        result.node->first = std::move(first);
        result.node->second = std::move(second);
        result.node->editor_name.clear();
    }

    ApplyLayout();
    return true;
}

void
SplitLayoutImpl::Update(float delta_time)
{
    if (!root_ || !editor_host_)
    {
        return;
    }

    UpdateRecursive(root_.get(), delta_time);
}

void
SplitLayoutImpl::UpdateRecursive(const SplitNode* node, float delta_time)
{
    if (!node)
    {
        return;
    }

    if (node->IsLeaf())
    {
        // Leaf — обновляем редактор
        if (!node->editor_name.empty())
        {
            std::string instance_id = MakeInstanceId(node);
            editor_host_->UpdateEditor(instance_id, delta_time);
        }
    }
    else
    {
        // Split — рекурсивно обновляем дочерние
        UpdateRecursive(node->first.get(), delta_time);
        UpdateRecursive(node->second.get(), delta_time);
    }
}

void
SplitLayoutImpl::Initialize()
{
    if (!root_ || !editor_host_)
    {
        return;
    }

    InitializeRecursive(root_.get());
    ApplyLayout();
}

void
SplitLayoutImpl::InitializeRecursive(const SplitNode* node)
{
    if (!node)
    {
        return;
    }

    if (node->IsLeaf())
    {
        if (!node->editor_name.empty())
        {
            std::string instance_id = MakeInstanceId(node);
            if (editor_host_->CreateEditor(node->editor_name, instance_id))
            {
                editor_host_->ActivateEditor(instance_id);
            }
        }
    }
    else
    {
        InitializeRecursive(node->first.get());
        InitializeRecursive(node->second.get());
    }
}

void
SplitLayoutImpl::DestroyRecursive(const SplitNode* node)
{
    if (!node)
    {
        return;
    }

    if (node->IsLeaf())
    {
        if (!node->editor_name.empty())
        {
            std::string instance_id = MakeInstanceId(node);
            editor_host_->DestroyEditor(instance_id);
        }
    }
    else
    {
        DestroyRecursive(node->first.get());
        DestroyRecursive(node->second.get());
    }
}

void
SplitLayoutImpl::ApplyLayout()
{
    if (!context_ || !root_)
    {
        return;
    }

    std::string rml = GenerateRML();
    Rml::Log::Message(Rml::Log::LT_INFO, "SplitLayout applied:\n%s", rml.c_str());
}

std::string
SplitLayoutImpl::GenerateRML() const
{
    if (!root_)
    {
        return {};
    }

    std::string output;
    GenerateRMLRecursive(root_.get(), output, 0);
    return output;
}

void
SplitLayoutImpl::GenerateRMLRecursive(const SplitNode* node, std::string& output, int depth) const
{
    if (!node)
    {
        return;
    }

    std::string indent(depth * 4, ' ');

    if (node->IsSplit())
    {
        const char* dir_attr = (node->direction == SplitDirection::Horizontal)
            ? "horizontal" : "vertical";

        output += indent + "<div class=\"split-container\" data-direction=\"" + dir_attr + "\"";
        output += " data-ratio=\"" + std::to_string(node->ratio) + "\">\n";

        if (node->first)
        {
            output += indent + "    <div class=\"split-panel split-first\">\n";
            GenerateRMLRecursive(node->first.get(), output, depth + 2);
            output += indent + "    </div>\n";
        }

        output += indent + "    <div class=\"split-divider\"></div>\n";

        if (node->second)
        {
            output += indent + "    <div class=\"split-panel split-second\">\n";
            GenerateRMLRecursive(node->second.get(), output, depth + 2);
            output += indent + "    </div>\n";
        }

        output += indent + "</div>\n";
    }
    else
    {
        // Leaf — панель с редактором
        if (!node->editor_name.empty())
        {
            std::string instance_id = MakeInstanceId(node);
            output += indent + "<div class=\"editor-panel\" data-editor=\"" + node->editor_name + "\"";
            output += " data-instance=\"" + instance_id + "\"";
            output += " data-min-size=\"" + std::to_string(node->min_size) + "\">\n";
            output += indent + "    <!-- Editor: " + node->editor_name + " -->\n";
            output += indent + "</div>\n";
        }
    }
}

SplitLayoutImpl::FindResult
SplitLayoutImpl::FindNode(SplitNode* root, const SplitNode* target)
{
    if (!root || !target)
    {
        return {};
    }

    if (root == target)
    {
        return {root, nullptr, false};
    }

    return FindNodeRecursive(root, nullptr, false, target);
}

SplitLayoutImpl::FindResult
SplitLayoutImpl::FindNodeRecursive(
    SplitNode* current,
    SplitNode* parent,
    bool is_first,
    const SplitNode* target)
{
    if (!current)
    {
        return {};
    }

    if (current == target)
    {
        return {current, parent, is_first};
    }

    if (current->first)
    {
        auto result = FindNodeRecursive(current->first.get(), current, true, target);
        if (result.node)
        {
            return result;
        }
    }

    if (current->second)
    {
        auto result = FindNodeRecursive(current->second.get(), current, false, target);
        if (result.node)
        {
            return result;
        }
    }

    return {};
}

std::string
SplitLayoutImpl::MakeInstanceId(const SplitNode* node) const
{
    // Используем адрес узла как уникальный идентификатор
    // Это работает пока дерево не перестраивается
    return node->editor_name + "_" + std::to_string(reinterpret_cast<uintptr_t>(node));
}

} // namespace skif::rmlui
