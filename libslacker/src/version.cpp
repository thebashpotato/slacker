#include <slacker/version.hpp>

namespace slacker {
    auto get_version() -> std::string {
        return std::string{MAJOR}.append(".").append(MINOR).append(".").append(
                PATCH);
    }
}// namespace slacker
