#ifndef SLACKER_LOCK_UTILS_VERSION_HPP_
#define SLACKER_LOCK_UTILS_VERSION_HPP_

/**
 * @file slacker_lock/utils/version.hpp
 * @author Matt Williams (matt.k.williams@protonmail.com)
 * @brief Adds version support for project
 * */

#include <string>

namespace slacker_lock::utils {
    constexpr auto VMAJOR = "0";
    constexpr auto VMINOR = "1";
    constexpr auto VPATCH = "0";

    /**
     * @brief return the version in concatenated format
     *
     * @returns std::string
     * */
    [[maybe_unused]] inline auto getVersion() -> std::string {
        return std::string{VMAJOR}.append(".").append(VMINOR).append(".").append(
                VPATCH);
    }
}// namespace slacker_lock::utils

#endif
