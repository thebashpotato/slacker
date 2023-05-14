#include <iostream>
#include <slacker.hpp>


/**
 * @brief A scratch file for interactively testing code
 * */
auto main() -> int {
    auto display = slacker::X11Display::open();
    if (!display) {
        std::cerr << "Failed to open X Display" << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "X11 screen_id count: " << display->screen_count() << '\n';
    std::cout << "X11 screen_id: " << display->screen_id() << '\n';
    std::cout << "X11 default root: " << display->default_root() << '\n';
    std::cout << "X11 root: " << display->root() << '\n';
    std::cout << "X11 server vendor: " << display->serverVendor() << '\n';

    return EXIT_SUCCESS;
}
