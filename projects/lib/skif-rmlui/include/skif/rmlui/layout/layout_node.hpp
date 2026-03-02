#pragma once

#include <skif/rmlui/config.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace skif::rmlui
{

/**
 * @brief Направление разделения панелей
 */
enum class SplitDirection
{
    Horizontal, // Панели слева/справа
    Vertical    // Панели сверху/снизу
};

/**
 * @brief Узел раскладки (панель или сплиттер)
 */
struct LayoutNode
{
    std::string view_name;        // Имя view или пусто для сплиттера
    float       ratio    = 0.5f;  // Соотношение размеров (0.0 - 1.0)
    float       min_size = 50.0f; // Минимальный размер

    std::unique_ptr<LayoutNode> first;  // Первая панель
    std::unique_ptr<LayoutNode> second; // Вторая панель
    SplitDirection              direction = SplitDirection::Horizontal;

    bool is_splitter = false; // Это сплиттер (не панель)

    LayoutNode() = default;

    // Конструктор для панели с view
    explicit LayoutNode(std::string_view view_name)
        : view_name(view_name)
    {
    }

    // Конструктор для сплиттера
    static std::unique_ptr<LayoutNode>
    CreateSplitter(
            SplitDirection              dir,
            float                       ratio  = 0.5f,
            std::unique_ptr<LayoutNode> first  = nullptr,
            std::unique_ptr<LayoutNode> second = nullptr
    )
    {
        auto node         = std::make_unique<LayoutNode>();
        node->is_splitter = true;
        node->direction   = dir;
        node->ratio       = ratio;
        node->first       = std::move(first);
        node->second      = std::move(second);
        return node;
    }

    // Конструктор для панели с view
    static std::unique_ptr<LayoutNode>
    CreatePanel(std::string_view view_name)
    {
        return std::make_unique<LayoutNode>(view_name);
    }
};

} // namespace skif::rmlui
