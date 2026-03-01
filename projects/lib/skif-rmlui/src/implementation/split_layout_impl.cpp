#include <implementation/split_layout_impl.hpp>
#include <implementation/panel_container_controller.hpp>
#include <implementation/editor_host_impl.hpp>

#include <skif/rmlui/view/lambda_event_listener.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Types.h>

#include <cassert>
#include <fstream>
#include <set>
#include <sstream>

namespace skif::rmlui
{

SplitLayoutImpl::SplitLayoutImpl() = default;

SplitLayoutImpl::~SplitLayoutImpl()
{
    if (root_ && editor_host_)
    {
        DestroyRecursive(root_.get());
    }
    CloseLayoutDocument();
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

    CloseLayoutDocument();
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

    // Копируем имена ДО модификации узла — string_view может ссылаться на editor_name этого узла
    std::string old_editor_name = result.node->editor_name;
    std::string new_editor_name_str(new_editor_name);

    result.node->editor_name.clear();
    result.node->direction = direction;
    result.node->ratio = ratio;
    result.node->first = SplitNode::MakeLeaf(old_editor_name);
    result.node->second = SplitNode::MakeLeaf(new_editor_name_str);

    // Пересоздаём layout — создаёт новые editor instances
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

    // Копируем new_editor_name — string_view может ссылаться на editor_name этого узла
    std::string new_name_str(new_editor_name);

    // Уничтожаем текущий редактор
    std::string old_instance_id = MakeInstanceId(result.node);
    editor_host_->DestroyEditor(old_instance_id);

    // Обновляем имя редактора в узле
    result.node->editor_name = new_name_str;

    // Пересоздаём layout — создаст новый editor instance
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

    // Обновляем status bar для всех контроллеров
    for (auto& controller : panel_controllers_)
    {
        controller->UpdateStatusBar();
    }
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

    // Генерируем и загружаем layout RML
    // ApplyLayout создаёт editor instances через CreateEditorEmbedded
    ApplyLayout();
}

void
SplitLayoutImpl::InitializeRecursive(const SplitNode* node)
{
    // Этот метод больше не используется напрямую —
    // editor instances создаются в LoadEditorContentRecursive
    if (!node)
    {
        return;
    }

    if (node->IsLeaf())
    {
        // Ничего — создание происходит в LoadEditorContentRecursive
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

// ============================================================================
// Layout document management
// ============================================================================

void
SplitLayoutImpl::ApplyLayout()
{
    if (!context_ || !root_)
    {
        return;
    }

    // Уничтожаем все существующие editor instances
    if (editor_host_)
    {
        editor_host_->DestroyAll();
    }

    // Закрыть предыдущий layout документ
    CloseLayoutDocument();

    // Генерируем полный RML документ
    std::string rml = GenerateRML();

    Rml::Log::Message(Rml::Log::LT_INFO, "SplitLayout: Loading layout RML (%zu bytes).", rml.size());

    // Загружаем через LoadDocumentFromMemory
    layout_document_ = context_->LoadDocumentFromMemory(rml, "layout");
    if (!layout_document_)
    {
        Rml::Log::Message(Rml::Log::LT_ERROR, "SplitLayout: Failed to load layout RML.");
        return;
    }

    // Загружаем Editor RML контент в panel-content div для каждого leaf
    LoadEditorContentRecursive(root_.get());

    // Создаём PanelContainerController для каждого leaf
    SetupPanelControllersRecursive(root_.get());

    // Привязываем drag-обработчики к dividers
    SetupDividerHandlers();

    // Показываем layout документ
    layout_document_->Show();

    Rml::Log::Message(Rml::Log::LT_INFO, "SplitLayout: Layout applied successfully.");
}

void
SplitLayoutImpl::CloseLayoutDocument()
{
    panel_controllers_.clear();
    dragging_divider_ = nullptr;
    dragging_split_node_ = nullptr;

    if (layout_document_)
    {
        layout_document_->Close();
        layout_document_ = nullptr;
    }
}

// ============================================================================
// RML generation
// ============================================================================

std::string
SplitLayoutImpl::GenerateRML() const
{
    if (!root_)
    {
        return {};
    }

    std::string output;

    // Собираем уникальные RCSS пути из дескрипторов Editor'ов
    std::set<std::string> rcss_paths;
    CollectRcssPaths(root_.get(), rcss_paths);

    // Генерируем полный RML документ с head и стилями
    output += "<rml>\n";
    output += "<head>\n";
    output += "    <title>Layout</title>\n";
    output += "    <link type=\"text/rcss\" href=\"assets/ui/panel_container.rcss\"/>\n";

    // Подключаем RCSS файлы Editor'ов
    for (const auto& rcss : rcss_paths)
    {
        output += "    <link type=\"text/rcss\" href=\"" + rcss + "\"/>\n";
    }

    output += "    <style>\n";
    output += "        body {\n";
    output += "            width: 100%;\n";
    output += "            height: 100%;\n";
    output += "            margin: 0;\n";
    output += "            padding: 0;\n";
    output += "            font-family: \"IBM Plex Mono\";\n";
    output += "            font-size: 14dp;\n";
    output += "            color: #ffffff;\n";
    output += "            background-color: #1a1a1a;\n";
    output += "        }\n";
    output += "    </style>\n";
    output += "</head>\n";
    output += "<body>\n";

    GenerateRMLRecursive(root_.get(), output, 1);

    output += "</body>\n";
    output += "</rml>\n";

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

        // Split container с data-node-id для поиска при divider drag
        std::string node_id = std::to_string(reinterpret_cast<uintptr_t>(node));

        output += indent + "<div class=\"split-container\" data-direction=\"" + dir_attr + "\"";
        output += " data-node-id=\"" + node_id + "\"";
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

        // Divider с data-node-id для привязки к SplitNode
        output += indent + "    <div class=\"split-divider\" data-direction=\"" + dir_attr + "\"";
        output += " data-node-id=\"" + node_id + "\"></div>\n";

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

    // Content area — Editor RML будет вставлен сюда через SetInnerRML
    output += indent + "    <div class=\"panel-content\" data-instance=\"" + instance_id + "\"></div>\n";

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
// Editor content loading
// ============================================================================

void
SplitLayoutImpl::LoadEditorContentRecursive(const SplitNode* node)
{
    if (!node || !layout_document_ || !editor_host_)
    {
        return;
    }

    if (node->IsLeaf())
    {
        if (node->editor_name.empty())
        {
            Rml::Log::Message(Rml::Log::LT_WARNING,
                "SplitLayout: Skipping leaf with empty editor_name (node=%p).",
                static_cast<const void*>(node));
            return;
        }

        std::string instance_id = MakeInstanceId(node);

        Rml::Log::Message(Rml::Log::LT_INFO,
            "SplitLayout: Loading editor content for '%s' (instance '%s').",
            node->editor_name.c_str(), instance_id.c_str());

        // Находим panel-content элемент по data-instance
        Rml::Element* content_element = layout_document_->QuerySelector(
            ".panel-content[data-instance=\"" + instance_id + "\"]"
        );

        if (!content_element)
        {
            Rml::Log::Message(Rml::Log::LT_ERROR,
                "SplitLayout: panel-content not found for instance '%s'.",
                instance_id.c_str());
            return;
        }

        // Создаём editor через EditorHostImpl::CreateEditorEmbedded
        auto* editor_host_impl = dynamic_cast<EditorHostImpl*>(editor_host_);
        if (editor_host_impl)
        {
            if (editor_host_impl->CreateEditorEmbedded(
                    node->editor_name, instance_id, content_element, layout_document_))
            {
                editor_host_impl->ActivateEditor(instance_id);
            }
        }
        else
        {
            // Fallback: legacy CreateEditor
            if (editor_host_->CreateEditor(node->editor_name, instance_id))
            {
                editor_host_->ActivateEditor(instance_id);
            }
        }
    }
    else
    {
        LoadEditorContentRecursive(node->first.get());
        LoadEditorContentRecursive(node->second.get());
    }
}

// ============================================================================
// Panel controllers
// ============================================================================

void
SplitLayoutImpl::SetupPanelControllersRecursive(const SplitNode* node)
{
    if (!node || !layout_document_)
    {
        return;
    }

    if (node->IsLeaf())
    {
        if (node->editor_name.empty())
        {
            return;
        }

        std::string instance_id = MakeInstanceId(node);

        // Находим panel-container элемент
        Rml::Element* container = layout_document_->QuerySelector(
            ".panel-container[data-instance=\"" + instance_id + "\"]"
        );

        if (!container)
        {
            Rml::Log::Message(Rml::Log::LT_WARNING,
                "SplitLayout: panel-container not found for instance '%s'.",
                instance_id.c_str());
            return;
        }

        auto controller = std::make_unique<PanelContainerController>(
            *this, *editor_host_, *editor_registry_,
            node, container, instance_id
        );
        controller->BindEvents();
        panel_controllers_.push_back(std::move(controller));
    }
    else
    {
        SetupPanelControllersRecursive(node->first.get());
        SetupPanelControllersRecursive(node->second.get());
    }
}

// ============================================================================
// Divider drag
// ============================================================================

void
SplitLayoutImpl::SetupDividerHandlers()
{
    if (!layout_document_)
    {
        return;
    }

    Rml::ElementList dividers;
    layout_document_->QuerySelectorAll(dividers, ".split-divider");
    for (int i = 0; i < static_cast<int>(dividers.size()); ++i)
    {
        Rml::Element* divider = dividers[i];

        BindEvent(divider, "mousedown",
            [this, divider](Rml::Event& event) { OnDividerMouseDown(event, divider); }
        );
    }

    // Document-level mousemove и mouseup для drag
    BindEvent(layout_document_, "mousemove",
        [this](Rml::Event& event) { OnDocumentMouseMove(event); }
    );

    BindEvent(layout_document_, "mouseup",
        [this](Rml::Event& event) { OnDocumentMouseUp(event); }
    );
}

void
SplitLayoutImpl::OnDividerMouseDown(Rml::Event& event, Rml::Element* divider)
{
    if (!divider)
    {
        return;
    }

    dragging_divider_ = divider;

    // Определяем направление
    Rml::String direction = divider->GetAttribute<Rml::String>("data-direction", "horizontal");
    bool is_horizontal = (direction == "horizontal");

    // Получаем начальную позицию мыши
    if (is_horizontal)
    {
        drag_start_mouse_ = static_cast<float>(event.GetParameter<int>("mouse_x", 0));
    }
    else
    {
        drag_start_mouse_ = static_cast<float>(event.GetParameter<int>("mouse_y", 0));
    }

    // Находим соответствующий SplitNode
    dragging_split_node_ = FindSplitNodeForDivider(divider);
    if (dragging_split_node_)
    {
        drag_start_ratio_ = dragging_split_node_->ratio;

        // Получаем размер контейнера для расчёта нового ratio
        Rml::Element* parent = divider->GetParentNode();
        if (parent)
        {
            auto box = parent->GetBox();
            if (is_horizontal)
            {
                drag_container_size_ = parent->GetClientWidth();
            }
            else
            {
                drag_container_size_ = parent->GetClientHeight();
            }
        }
    }

    Rml::Log::Message(Rml::Log::LT_INFO,
        "SplitLayout: Divider drag started (ratio=%.2f).", drag_start_ratio_);
}

void
SplitLayoutImpl::OnDocumentMouseMove(Rml::Event& event)
{
    if (!dragging_divider_ || !dragging_split_node_)
    {
        return;
    }

    Rml::String direction = dragging_divider_->GetAttribute<Rml::String>("data-direction", "horizontal");
    bool is_horizontal = (direction == "horizontal");

    float current_mouse;
    if (is_horizontal)
    {
        current_mouse = static_cast<float>(event.GetParameter<int>("mouse_x", 0));
    }
    else
    {
        current_mouse = static_cast<float>(event.GetParameter<int>("mouse_y", 0));
    }

    if (drag_container_size_ <= 0.0f)
    {
        return;
    }

    float delta = current_mouse - drag_start_mouse_;
    float delta_ratio = delta / drag_container_size_;
    float new_ratio = drag_start_ratio_ + delta_ratio;

    // Clamp ratio
    float min_ratio = dragging_split_node_->min_size / drag_container_size_;
    float max_ratio = 1.0f - min_ratio;
    if (new_ratio < min_ratio) new_ratio = min_ratio;
    if (new_ratio > max_ratio) new_ratio = max_ratio;

    dragging_split_node_->ratio = new_ratio;

    // Обновляем CSS размеры панелей
    Rml::Element* parent = dragging_divider_->GetParentNode();
    if (parent)
    {
        int first_pct = static_cast<int>(new_ratio * 100.0f);
        int second_pct = 100 - first_pct;

        const char* size_prop = is_horizontal ? "width" : "height";

        // Находим split-first и split-second среди прямых потомков
        Rml::Element* first_panel = nullptr;
        Rml::Element* second_panel = nullptr;
        for (int ci = 0; ci < parent->GetNumChildren(); ++ci)
        {
            Rml::Element* child = parent->GetChild(ci);
            if (child->IsClassSet("split-first"))
                first_panel = child;
            else if (child->IsClassSet("split-second"))
                second_panel = child;
        }

        if (first_panel)
        {
            first_panel->SetProperty(size_prop, std::to_string(first_pct) + "%");
        }
        if (second_panel)
        {
            second_panel->SetProperty(size_prop, std::to_string(second_pct) + "%");
        }
    }
}

void
SplitLayoutImpl::OnDocumentMouseUp(Rml::Event& /*event*/)
{
    if (dragging_divider_)
    {
        Rml::Log::Message(Rml::Log::LT_INFO,
            "SplitLayout: Divider drag ended (ratio=%.2f).",
            dragging_split_node_ ? dragging_split_node_->ratio : 0.0f);
    }

    dragging_divider_ = nullptr;
    dragging_split_node_ = nullptr;
}

SplitNode*
SplitLayoutImpl::FindSplitNodeForDivider(Rml::Element* divider)
{
    if (!divider || !root_)
    {
        return nullptr;
    }

    // Divider имеет data-node-id, совпадающий с адресом SplitNode
    Rml::String node_id_str = divider->GetAttribute<Rml::String>("data-node-id", "");
    if (node_id_str.empty())
    {
        return nullptr;
    }

    // Рекурсивно ищем SplitNode по адресу
    uintptr_t target_addr = 0;
    try
    {
        target_addr = std::stoull(node_id_str);
    }
    catch (...)
    {
        return nullptr;
    }

    return FindSplitNodeByAddress(root_.get(), target_addr);
}

// ============================================================================
// Node search helpers
// ============================================================================

SplitNode*
SplitLayoutImpl::FindSplitNodeByAddress(SplitNode* node, uintptr_t address)
{
    if (!node)
    {
        return nullptr;
    }

    if (reinterpret_cast<uintptr_t>(node) == address)
    {
        return node;
    }

    if (node->first)
    {
        auto* result = FindSplitNodeByAddress(node->first.get(), address);
        if (result) return result;
    }

    if (node->second)
    {
        auto* result = FindSplitNodeByAddress(node->second.get(), address);
        if (result) return result;
    }

    return nullptr;
}

SplitNode*
SplitLayoutImpl::FindNodeByInstanceId(std::string_view instance_id)
{
    if (!root_)
    {
        return nullptr;
    }
    return FindNodeByInstanceIdRecursive(root_.get(), instance_id);
}

SplitNode*
SplitLayoutImpl::FindNodeByInstanceIdRecursive(SplitNode* node, std::string_view instance_id)
{
    if (!node)
    {
        return nullptr;
    }

    if (node->IsLeaf())
    {
        if (MakeInstanceId(node) == instance_id)
        {
            return node;
        }
    }
    else
    {
        auto* result = FindNodeByInstanceIdRecursive(node->first.get(), instance_id);
        if (result) return result;
        result = FindNodeByInstanceIdRecursive(node->second.get(), instance_id);
        if (result) return result;
    }

    return nullptr;
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

void
SplitLayoutImpl::CollectRcssPaths(const SplitNode* node, std::set<std::string>& paths) const
{
    if (!node)
    {
        return;
    }

    if (node->IsLeaf())
    {
        if (!node->editor_name.empty() && editor_registry_)
        {
            const EditorDescriptor* desc = editor_registry_->GetDescriptor(node->editor_name);
            if (desc && !desc->rcss_path.empty())
            {
                paths.insert(desc->rcss_path);
            }
        }
    }
    else
    {
        CollectRcssPaths(node->first.get(), paths);
        CollectRcssPaths(node->second.get(), paths);
    }
}

std::string
SplitLayoutImpl::MakeInstanceId(const SplitNode* node) const
{
    return node->editor_name + "_" + std::to_string(reinterpret_cast<uintptr_t>(node));
}

} // namespace skif::rmlui
