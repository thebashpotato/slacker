#include <iostream>
#include <slacker_lock/utils/version.hpp>

/**
 * @brief A scratch file for interactively testing code
 * */
int main() {
  std::cout << "Slack Lock: " << slacker_lock::get_version() << '\n';
  return 0;
}
