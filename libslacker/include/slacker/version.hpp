#ifndef SLACKER_VERSION_HPP
#define SLACKER_VERSION_HPP

#include <string>

namespace slacker {
    constexpr auto MAJOR = "0";
    constexpr auto MINOR = "1";
    constexpr auto PATCH = "0";

    /**
     * @brief return the version in concatenated format
     *
     * @returns std::string
     * */
    [[nodiscard]] auto get_version() -> std::string;
}// namespace slacker
#endif