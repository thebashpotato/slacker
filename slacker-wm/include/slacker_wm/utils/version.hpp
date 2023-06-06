#ifndef SLACKER_WM_UTILS_VERSION_HPP
#define SLACKER_WM_UTILS_VERSION_HPP

#include <string>

namespace slacker_wm::utils {
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
}// namespace slacker_wm::utils

#endif
