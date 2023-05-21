#ifndef SLACKER_UTILS_VERSION_HPP
#define SLACKER_UTILS_VERSION_HPP

#include <string>

namespace slacker::utils {
    constexpr auto MAJOR = "0";
    constexpr auto MINOR = "1";
    constexpr auto PATCH = "0";

    /**
     * @brief return the version in concatenated format
     *
     * @returns std::string
     * */
    [[nodiscard]] inline auto getVersion() -> std::string {
        return std::string{MAJOR}.append(".").append(MINOR).append(".").append(
                PATCH);
    }
}// namespace slacker::utils

#endif // SLACKER_UTILS_VERSION_HPP
