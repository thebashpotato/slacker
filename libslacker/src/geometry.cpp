#include <slacker/geometry.hpp>

namespace slacker {
    constexpr auto WINDOW_XAXIS_POS = 500;
    constexpr auto WINDOW_YAXIS_POS = 500;
    constexpr auto WINDOW_WIDTH = 500;
    constexpr auto WINDOW_HEIGHT = 500;
    constexpr auto WINDOW_BORDER = 15;

    Rect::Rect()
        : xaxis_pos(WINDOW_XAXIS_POS), yaxis_pos(WINDOW_YAXIS_POS), width(WINDOW_WIDTH), height(WINDOW_HEIGHT), border(WINDOW_BORDER) {}

    [[maybe_unused]] Rect::Rect(int32_t pos_x, int32_t pos_y,
                                uint32_t window_width, uint32_t window_height,
                                uint32_t window_border)
        : xaxis_pos(pos_x), yaxis_pos(pos_y), width(window_width),
          height(window_height), border(window_border) {}

}// namespace slacker