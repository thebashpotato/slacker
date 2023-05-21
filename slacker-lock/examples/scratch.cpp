#include "slacker_lock/utils/version.hpp"
#include <iostream>

/**
 * @brief A scratch file for interactively testing code
 * */
auto main() -> int {
    std::cout << "Slack Lock: " << slacker_lock::utils::getVersion() << '\n';
    return 0;
}
