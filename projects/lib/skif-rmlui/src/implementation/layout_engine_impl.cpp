#include <implementation/layout_engine_impl.hpp>

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Log.h>

namespace skif::rmlui
{

LayoutEngineImpl::LayoutEngineImpl() = default;

void
LayoutEngineImpl::SetContext(Rml::Context* context)
{
    context_ = context;
}

void
LayoutEngineImpl::SetRoot(std::unique_ptr<LayoutNode> root)
{
    root_ = std::move(root);
    ApplyLayout();
}

const LayoutNode*
LayoutEngineImpl::GetRoot() const
{
    return root_.get();
}

bool
LayoutEngineImpl::SplitPanel(Rml::Element* panel, SplitDirection direction, float ratio)
{
    if (!panel || !root_)
    {
        return false;
    }

    // Найти существующий узел для этой панели
    LayoutNode* node = FindNodeByElement(root_.get(), panel);
    if (!node || node->is_splitter)
    {
        return false;
    }

    // Создать новый сплиттер
    auto splitter = std::make_unique<LayoutNode>();
    splitter->is_splitter = true;
    splitter->direction = direction;
    splitter->ratio = ratio;

    // Создать новую панель (пустую)
    auto new_panel = std::make_unique<LayoutNode>();
    
    // Переместить текущую панель в first
    splitter->first = std::make_unique<LayoutNode>(node->view_name);
    splitter->first->view_name = node->view_name;
    
    // Новую панель в second
    splitter->second = std::move(new_panel);

    // Найти родителя и заменить
    // TODO: implement parent replacement
    
    ApplyLayout();
    return true;
}

bool
LayoutEngineImpl::MergePanels(Rml::Element* /*first*/, Rml::Element* /*second*/)
{
    // TODO: implement merge
    return false;
}

void
LayoutEngineImpl::BeginDrag(Rml::Element* splitter)
{
    if (!splitter)
    {
        return;
    }

    dragging_splitter_ = splitter;
    dragging_node_ = FindNodeByElement(root_.get(), splitter);
    
    if (dragging_node_)
    {
        drag_start_ratio_ = dragging_node_->ratio;
    }
}

void
LayoutEngineImpl::UpdateDrag(float /*mouse_x*/, float /*mouse_y*/)
{
    // TODO: calculate new ratio based on mouse position
    if (dragging_node_)
    {
        // dragging_node_->ratio = ...
        ApplyLayout();
    }
}

void
LayoutEngineImpl::EndDrag()
{
    dragging_splitter_ = nullptr;
    dragging_node_ = nullptr;
}

std::string
LayoutEngineImpl::GenerateRML() const
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
LayoutEngineImpl::GenerateRMLRecursive(const LayoutNode* node, std::string& output, int depth) const
{
    if (!node)
    {
        return;
    }

    std::string indent(depth * 4, ' ');
    
    if (node->is_splitter)
    {
        // Сплиттер - создаём контейнер
        const char* direction_attr = (node->direction == SplitDirection::Horizontal) 
            ? "horizontal" : "vertical";
        
        output += indent + "<div id=\"splitter\" data-split=\"" + std::string(direction_attr) + "\"";
        output += " data-ratio=\"" + std::to_string(node->ratio) + "\">\n";
        
        if (node->first)
        {
            GenerateRMLRecursive(node->first.get(), output, depth + 1);
        }
        
        if (node->second)
        {
            GenerateRMLRecursive(node->second.get(), output, depth + 1);
        }
        
        output += indent + "</div>\n";
    }
    else
    {
        // Панель - создаём элемент для view
        if (!node->view_name.empty())
        {
            output += indent + "<div id=\"panel_" + node->view_name + "\"";
            output += " data-view=\"" + node->view_name + "\"";
            output += " data-min-size=\"" + std::to_string(node->min_size) + "\">\n";
            output += indent + "  <!-- View: " + node->view_name + " -->\n";
            output += indent + "</div>\n";
        }
    }
}

void
LayoutEngineImpl::ApplyLayout()
{
    if (!context_ || !root_)
    {
        return;
    }

    // Генерируем RML для раскладки
    std::string rml = GenerateRML();
    
    // Загружаем в контекст (в реальной реализации это было бы более сложно)
    Rml::Log::Message(Rml::Log::LT_INFO, "Layout applied:\n%s", rml.c_str());
}

LayoutNode*
LayoutEngineImpl::FindNodeByElement(LayoutNode* node, Rml::Element* element)
{
    if (!node || !element)
    {
        return nullptr;
    }

    auto it = element_to_node_.find(element);
    if (it != element_to_node_.end())
    {
        return it->second;
    }

    // Рекурсивный поиск
    if (node->first)
    {
        if (auto* found = FindNodeByElement(node->first.get(), element))
        {
            return found;
        }
    }
    
    if (node->second)
    {
        if (auto* found = FindNodeByElement(node->second.get(), element))
        {
            return found;
        }
    }

    return nullptr;
}

void
LayoutEngineImpl::UpdatePanelSizes(LayoutNode* /*node*/, float /*width*/, float /*height*/)
{
    // TODO: рассчитать размеры панелей на основе ratio и контейнера
}

} // namespace skif::rmlui