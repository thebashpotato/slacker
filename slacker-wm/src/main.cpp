#include "slacker_wm/utils/version.hpp"
#include <cstdlib>
#include <iostream>

auto main() -> int {
    std::cout << "Slacker Lock Version: " << slacker_wm::utils::getVersion() << '\n';

    return EXIT_SUCCESS;
}
