#pragma once

#include <skif/rmlui/layout/i_layout_engine.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// Forward declaration RmlUi
namespace Rml
{
class Context;
class Element;
} // namespace Rml

namespace skif::rmlui
{

/**
 * @brief Реализация движка раскладки
 */
class LayoutEngineImpl final : public ILayoutEngine
{
  public:

    explicit LayoutEngineImpl();
    ~LayoutEngineImpl() override = default;

    // ILayoutEngine
    void                            SetContext(Rml::Context *context) override;
    void                            SetRoot(std::unique_ptr<LayoutNode> root) override;
    [[nodiscard]] const LayoutNode *GetRoot() const override;
    bool                            SplitPanel(Rml::Element *panel, SplitDirection direction, float ratio = 0.5f) override;
    bool                            MergePanels(Rml::Element *first, Rml::Element *second) override;
    void                            BeginDrag(Rml::Element *splitter) override;
    void                            UpdateDrag(float mouse_x, float mouse_y) override;
    void                            EndDrag() override;
    std::string                     GenerateRML() const override;
    void                            ApplyLayout() override;

  private:

    // Найти узел по элементу
    LayoutNode *FindNodeByElement(LayoutNode *node, Rml::Element *element);

    // Обновить размеры панелей
    void UpdatePanelSizes(LayoutNode *node, float width, float height);

    // Сгенерировать RML для узла
    void GenerateRMLRecursive(const LayoutNode *node, std::string &output, int depth) const;

    Rml::Context               *context_ = nullptr;
    std::unique_ptr<LayoutNode> root_;

    // Drag state
    Rml::Element *dragging_splitter_ = nullptr;
    LayoutNode   *dragging_node_     = nullptr;
    float         drag_start_ratio_  = 0.5f;

    // Mapping от элементов к узлам
    std::unordered_map<Rml::Element *, LayoutNode *> element_to_node_;
};

} // namespace skif::rmlui
