#include <skif/rmlui/app.hpp>

#include "plugins/sample_panel.hpp"

int
main(int argc, char *argv[])
{
    using namespace skif::rmlui;

    App app {argc, argv};

    // Регистрируем sample плагин (статическая регистрация)
    app.GetPluginManager().RegisterPlugin(std::make_unique<sample::SamplePanelPlugin>());

    // Устанавливаем начальное view и fallback RML
    app.SetInitialView("sample_panel");
    app.SetFallbackRml("assets/ui/basic.rml");

    return app.run();
}
