#include <cstdlib>
#include <iostream>
#include <slacker_lock/utils/version.hpp>
// #include <slacker.hpp>

auto main() -> int {
    std::cout << "Slacker Lock Version: " << slacker_lock::get_version() << '\n';

    return EXIT_SUCCESS;
}
