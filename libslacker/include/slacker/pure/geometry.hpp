#ifndef SLACKER_GEOMETRY_HPP
#define SLACKER_GEOMETRY_HPP

#include "slacker/utils/attributes.hpp"
#include <cstdint>

namespace slacker::pure {
    /**
     * @brief Default values for default constructor
     * */
    constexpr auto WINDOW_XAXIS_POS = 500;
    constexpr auto WINDOW_YAXIS_POS = 500;
    constexpr auto WINDOW_WIDTH = 500;
    constexpr auto WINDOW_HEIGHT = 500;
    constexpr auto WINDOW_BORDER = 15;

    /**
     * @brief Describes the position, height, width and border of a window.
     * */
    class SLACKER_EXPORT Rect {
    public:
        Rect() = default;

        [[maybe_unused]] Rect(int32_t pos_x, int32_t pos_y, int32_t width, int32_t height,
                              int32_t border);

    public:
        int32_t xaxis_pos{WINDOW_XAXIS_POS};
        int32_t yaxis_pos{WINDOW_YAXIS_POS};
        uint32_t width{WINDOW_WIDTH};
        uint32_t height{WINDOW_HEIGHT};
        uint32_t border{WINDOW_BORDER};
    };
}// namespace slacker::pure

#endif// SLACKER_GEOMETRY_HPP
