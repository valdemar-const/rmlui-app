#pragma once

namespace skif::rmlui
{
struct App
{
    using Return_Code = int;

    App(int argc, char *argv[]);

    Return_Code run(void);
};
} // namespace skif::rmlui
