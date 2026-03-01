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
SplitLayoutImpl::SetEditorRegistry(IEditorRegistry* registry)
{
    editor_registry_ = registry;
}

void
SplitLayoutImpl::SetRoot(std::unique_ptr<SplitNode> root)
{
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

    auto result = FindNode(root_.get(), panel);
    if (!result.node)
    {
        return false;
    }

    std::string old_editor_name = result.node->editor_name;

    result.node->editor_name.clear();
    result.node->direction = direction;
    result.node->ratio = ratio;
    result.node->first = SplitNode::MakeLeaf(old_editor_name);
    result.node->second = SplitNode::MakeLeaf(new_editor_name);

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

    SplitNode* keep_node = keep_first ? result.node->first.get() : result.node->second.get();
    SplitNode* remove_node = keep_first ? result.node->second.get() : result.node->first.get();

    if (editor_host_)
    {
        DestroyRecursive(remove_node);
    }

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
        auto first = std::move(keep_node->first);
        auto second = std::move(keep_node->second);
        result.node->first = std::move(first);
        result.node->second = std::move(second);
        result.node->editor_name.clear();
    }

    ApplyLayout();
    return true;
}

bool
SplitLayoutImpl::SwitchEditor(const SplitNode* panel, std::string_view new_editor_name)
{
    if (!panel || !root_ || !panel->IsLeaf() || !editor_host_)
    {
        return false;
    }

    auto result = FindNode(root_.get(), panel);
    if (!result.node || !result.node->IsLeaf())
    {
        return false;
    }

    // Уничтожаем текущий редактор
    std::string old_instance_id = MakeInstanceId(result.node);
    editor_host_->DestroyEditor(old_instance_id);

    // Обновляем имя редактора в узле
    result.node->editor_name = new_editor_name;

    // Создаём новый редактор
    std::string new_instance_id = MakeInstanceId(result.node);
    if (editor_host_->CreateEditor(new_editor_name, new_instance_id))
    {
        editor_host_->ActivateEditor(new_instance_id);
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
        if (!node->editor_name.empty())
        {
            std::string instance_id = MakeInstanceId(node);
            editor_host_->UpdateEditor(instance_id, delta_time);
        }
    }
    else
    {
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

        const char* flex_dir = (node->direction == SplitDirection::Horizontal)
            ? "row" : "column";

        const char* size_prop = (node->direction == SplitDirection::Horizontal)
            ? "width" : "height";

        int first_pct = static_cast<int>(node->ratio * 100.0f);
        int second_pct = 100 - first_pct;

        output += indent + "<div class=\"split-container\" data-direction=\"" + dir_attr + "\"";
        output += " style=\"display: flex; flex-direction: " + std::string(flex_dir) + "; width: 100%; height: 100%;\"";
        output += " data-ratio=\"" + std::to_string(node->ratio) + "\">\n";

        // First panel
        if (node->first)
        {
            output += indent + "    <div class=\"split-panel split-first\"";
            output += " style=\"" + std::string(size_prop) + ": " + std::to_string(first_pct) + "%;\">\n";
            GenerateRMLRecursive(node->first.get(), output, depth + 2);
            output += indent + "    </div>\n";
        }

        // Divider
        output += indent + "    <div class=\"split-divider\" data-direction=\"" + dir_attr + "\"></div>\n";

        // Second panel
        if (node->second)
        {
            output += indent + "    <div class=\"split-panel split-second\"";
            output += " style=\"" + std::string(size_prop) + ": " + std::to_string(second_pct) + "%;\">\n";
            GenerateRMLRecursive(node->second.get(), output, depth + 2);
            output += indent + "    </div>\n";
        }

        output += indent + "</div>\n";
    }
    else
    {
        // Leaf — генерируем panel container
        GeneratePanelContainerRML(node, output, depth);
    }
}

void
SplitLayoutImpl::GeneratePanelContainerRML(const SplitNode* node, std::string& output, int depth) const
{
    if (!node || node->editor_name.empty())
    {
        return;
    }

    std::string indent(depth * 4, ' ');
    std::string instance_id = MakeInstanceId(node);

    // Получаем дескриптор для display_name и menu
    const EditorDescriptor* descriptor = nullptr;
    if (editor_registry_)
    {
        descriptor = editor_registry_->GetDescriptor(node->editor_name);
    }

    std::string display_name = descriptor ? descriptor->display_name : node->editor_name;

    // Panel container
    output += indent + "<div class=\"panel-container\" data-instance=\"" + instance_id + "\"";
    output += " data-editor=\"" + node->editor_name + "\">\n";

    // Header bar
    output += indent + "    <div class=\"panel-header\">\n";

    // Editor switcher dropdown
    GenerateEditorSwitcherRML(node, output, depth + 2);

    // Menu entries from descriptor
    if (descriptor && !descriptor->menu_entries.empty())
    {
        output += indent + "        <div class=\"panel-menu\">\n";
        for (const auto& entry : descriptor->menu_entries)
        {
            output += indent + "            <button class=\"menu-item\" data-action=\"" + entry.action_id + "\">";
            output += entry.label;
            if (!entry.shortcut.empty())
            {
                output += " (" + entry.shortcut + ")";
            }
            output += "</button>\n";
        }
        output += indent + "        </div>\n";
    }

    output += indent + "    </div>\n";

    // Hot corners (top)
    output += indent + "    <div class=\"hot-corner hot-corner-tl\" data-action=\"split\" data-instance=\"" + instance_id + "\"></div>\n";
    output += indent + "    <div class=\"hot-corner hot-corner-tr\" data-action=\"split\" data-instance=\"" + instance_id + "\"></div>\n";

    // Content area
    output += indent + "    <div class=\"panel-content\" data-instance=\"" + instance_id + "\">\n";
    output += indent + "        <!-- Editor content: " + node->editor_name + " -->\n";
    output += indent + "    </div>\n";

    // Status bar
    output += indent + "    <div class=\"panel-statusbar\">\n";
    output += indent + "        <span class=\"status-text\">Ready</span>\n";
    output += indent + "    </div>\n";

    // Hot corners (bottom)
    output += indent + "    <div class=\"hot-corner hot-corner-bl\" data-action=\"merge\" data-instance=\"" + instance_id + "\"></div>\n";
    output += indent + "    <div class=\"hot-corner hot-corner-br\" data-action=\"merge\" data-instance=\"" + instance_id + "\"></div>\n";

    output += indent + "</div>\n";
}

void
SplitLayoutImpl::GenerateEditorSwitcherRML(const SplitNode* node, std::string& output, int depth) const
{
    std::string indent(depth * 4, ' ');
    std::string instance_id = MakeInstanceId(node);

    output += indent + "<select class=\"editor-switcher\" data-instance=\"" + instance_id + "\">\n";

    if (editor_registry_)
    {
        auto descriptors = editor_registry_->GetAllDescriptors();
        for (const auto* desc : descriptors)
        {
            output += indent + "    <option value=\"" + desc->name + "\"";
            if (desc->name == node->editor_name)
            {
                output += " selected";
            }
            output += ">" + desc->display_name + "</option>\n";
        }
    }
    else
    {
        // Fallback — только текущий редактор
        output += indent + "    <option value=\"" + node->editor_name + "\" selected>" + node->editor_name + "</option>\n";
    }

    output += indent + "</select>\n";
}

// ============================================================================
// Tree navigation
// ============================================================================

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
    return node->editor_name + "_" + std::to_string(reinterpret_cast<uintptr_t>(node));
}

} // namespace skif::rmlui
