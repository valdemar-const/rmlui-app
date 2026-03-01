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
    Horizontal,  // Панели слева/справа
    Vertical     // Панели сверху/снизу
};

/**
 * @brief Узел рекурсивного дерева панелей (SplitLayout)
 * @note Leaf — содержит имя редактора.
 *       Split — содержит два дочерних узла, направление и пропорцию.
 *       Обновление идёт рекурсивно от корня вниз.
 */
struct SplitNode
{
    /// Имя редактора (для leaf-узлов)
    std::string editor_name;

    /// Направление разделения (для split-узлов)
    SplitDirection direction = SplitDirection::Horizontal;

    /// Пропорция разделения (0.0 - 1.0, доля первого дочернего узла)
    float ratio = 0.5f;

    /// Минимальный размер панели в пикселях
    float min_size = 50.0f;

    /// Первый дочерний узел (nullptr для leaf)
    std::unique_ptr<SplitNode> first;

    /// Второй дочерний узел (nullptr для leaf)
    std::unique_ptr<SplitNode> second;

    SplitNode() = default;

    /// Проверить, является ли узел листом (содержит редактор)
    [[nodiscard]] bool IsLeaf() const noexcept
    {
        return !first && !second;
    }

    /// Проверить, является ли узел разделителем
    [[nodiscard]] bool IsSplit() const noexcept
    {
        return first && second;
    }

    /// Создать leaf-узел с редактором
    [[nodiscard]] static std::unique_ptr<SplitNode> MakeLeaf(std::string_view editor_name)
    {
        auto node = std::make_unique<SplitNode>();
        node->editor_name = editor_name;
        return node;
    }

    /// Создать split-узел с двумя дочерними
    [[nodiscard]] static std::unique_ptr<SplitNode> MakeSplit(
        SplitDirection dir,
        float ratio,
        std::unique_ptr<SplitNode> first,
        std::unique_ptr<SplitNode> second)
    {
        auto node = std::make_unique<SplitNode>();
        node->direction = dir;
        node->ratio = ratio;
        node->first = std::move(first);
        node->second = std::move(second);
        return node;
    }
};

} // namespace skif::rmlui
