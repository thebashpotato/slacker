#include "slacker_wm/utils/version.hpp"
#include <iostream>

/**
 * @brief A scratch file for interactively testing code
 * */
auto main() -> int {
    std::cout << "Slack Lock: " << slacker_wm::utils::getVersion() << '\n';
    return 0;
}
