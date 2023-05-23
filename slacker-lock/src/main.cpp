#include "slacker_lock/utils/version.hpp"
#include <cstdlib>
#include <iostream>

auto main() -> int {
    std::cout << "Slacker Lock Version: " << slacker_lock::utils::getVersion() << '\n';
    return EXIT_SUCCESS;
}
