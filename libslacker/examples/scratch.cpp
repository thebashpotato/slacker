#include <iostream>
#include <slacker.hpp>


/**
 * @brief A scratch file for interactively testing code
 * */
auto main() -> int {
    auto display = slacker::x::X11Display();
    if (!display.isOpen()) {
        std::cerr << "Failed to open X Display" << '\n';
        return EXIT_FAILURE;
    }

    std::cout << "X11 screen_id count: " << display.screenCount() << '\n';
    std::cout << "X11 screen_id: " << display.screenId() << '\n';
    std::cout << "X11 default root: " << display.defaultRoot() << '\n';
    std::cout << "X11 root: " << display.root() << '\n';
    std::cout << "X11 server vendor: " << display.serverVendor() << '\n';

    return EXIT_SUCCESS;
}
