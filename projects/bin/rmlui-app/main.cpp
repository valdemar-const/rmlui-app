#include <skif/rmlui/app.hpp>
#include <skif/rmlui/editor/split_node.hpp>

#include "plugins/sample_panel.hpp"

int
main(int argc, char *argv[])
{
    using namespace skif::rmlui;

    App app {argc, argv};

    // Регистрируем sample плагин (плагин регистрирует редакторы через contribution point)
    app.GetPluginManager().RegisterPlugin(std::make_unique<sample::SamplePanelPlugin>());

    // Устанавливаем начальный layout — одна панель с sample_panel
    app.SetInitialLayout(SplitNode::MakeLeaf("sample_panel"));

    // Fallback RML на случай если редактор не загрузится
    app.SetFallbackRml("assets/ui/basic.rml");

    return app.run();
}
