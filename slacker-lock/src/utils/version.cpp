#include <slacker_lock/utils/version.hpp>

namespace slacker_lock {
    auto get_version() -> std::string {
        return std::string{VMAJOR}.append(".").append(VMINOR).append(".").append(
                VPATCH);
    }
}// namespace slacker_lock
