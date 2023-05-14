#include <slacker/geometry.hpp>

namespace slacker {

    [[maybe_unused]] Rect::Rect(int32_t pos_x, int32_t pos_y,
                                uint32_t window_width, uint32_t window_height,
                                uint32_t window_border)
        : xaxis_pos(pos_x), yaxis_pos(pos_y), width(window_width),
          height(window_height), border(window_border) {}
}// namespace slacker