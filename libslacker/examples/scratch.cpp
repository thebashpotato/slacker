#include "slacker/x/display.hpp"
#include <iostream>
#include <slacker.hpp>


/**
 * @brief A scratch file for interactively testing code
 * */
auto main() -> int {
    auto result = slacker::x::X11Display::open();
    if (result.isErr()) {
        std::cerr << "Failed to open X Display" << '\n';
        return EXIT_FAILURE;
    }
    auto display = result.ok().value();

    std::cout << "X11 screen count: " << display->screenCount() << '\n';
    std::cout << "X11 screen id: " << display->screenId() << '\n';
    std::cout << "X11 default root window: " << display->defaultRoot() << '\n';
    std::cout << "X11 root window from screen id: " << display->root() << '\n';
    std::cout << "X11 server vendor: " << display->serverVendor() << '\n';

    return EXIT_SUCCESS;
}
