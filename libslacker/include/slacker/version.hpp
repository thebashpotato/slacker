#ifndef SLACKER_VERSION_HPP_
#define SLACKER_VERSION_HPP_

#include <string>

namespace slacker {
constexpr auto VMAJOR = "0";
constexpr auto VMINOR = "1";
constexpr auto VPATCH = "0";

/**
 * @brief return the version in concatenated format
 *
 * @returns std::string
 * */
[[maybe_unused]] auto get_version() -> std::string;
} // namespace slacker
#endif