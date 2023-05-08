#include <slacker/version.hpp>

namespace slacker {
auto get_version() -> std::string {
  return std::string{VMAJOR}.append(".").append(VMINOR).append(".").append(
      VPATCH);
}
} // namespace slacker
