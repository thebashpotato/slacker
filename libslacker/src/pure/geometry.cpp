#include "slacker/pure/geometry.hpp"

namespace slacker::pure {

    [[maybe_unused]] Rect::Rect(int32_t pos_x, int32_t pos_y,
                                int32_t window_width, int32_t window_height,
                                int32_t window_border)
        : xaxis_pos(pos_x), yaxis_pos(pos_y), width(window_width),
          height(window_height), border(window_border) {}
}// namespace slacker::pure
