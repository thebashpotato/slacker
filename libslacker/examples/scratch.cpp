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

    std::cout << "X11 server vendor: " << display->serverVendor() << '\n';

    return EXIT_SUCCESS;
}
