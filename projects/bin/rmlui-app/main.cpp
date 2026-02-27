#include <skif/rmlui/app.hpp>

#include "plugins/sample_panel.cpp"

int
main(int argc, char *argv[])
{
    using namespace skif::rmlui;

    App app {argc, argv};

    // Регистрируем sample плагин (статическая регистрация)
    app.GetPluginManager().RegisterPlugin(std::make_unique<sample::SamplePanelPlugin>());

    return app.run();
}
