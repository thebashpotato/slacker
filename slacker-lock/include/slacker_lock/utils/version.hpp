#ifndef SLACKER_LOCK_UTILS_VERSION_HPP_
#define SLACKER_LOCK_UTILS_VERSION_HPP_

/**
 * @file slacker_lock/utils/version.hpp
 * @author Matt Williams (matt.k.williams@protonmail.com)
 * @brief Adds version support for project
 * */

#include <string>

namespace slacker_lock {
    constexpr auto VMAJOR = "0";
    constexpr auto VMINOR = "1";
    constexpr auto VPATCH = "0";

    /**
 * @brief return the version in concatenated format
 *
 * @returns std::string
 * */
    [[maybe_unused]] auto get_version() -> std::string;
}// namespace slacker_lock

#endif
