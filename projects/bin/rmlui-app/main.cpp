#include <skif/rmlui/app.hpp>

int
main(int argc, char *argv[])
{
    using namespace skif::rmlui;

    App app {argc, argv};
    return app.run();
}
