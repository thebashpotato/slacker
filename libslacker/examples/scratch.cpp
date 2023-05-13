#include <iostream>
#include <slacker/version.hpp>

/**
 * @brief A scratch file for interactively testing code
 * */
auto main() -> int {
    std::cout << "Slack Lock: " << slacker::get_version() << '\n';
    return 0;
}
