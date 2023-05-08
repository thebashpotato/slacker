#include <slacker/geometry.hpp>

namespace slacker {

Rect::Rect()
    : xaxis_pos(500), yaxis_pos(500), width(500), height(500), border(15) {}

Rect::Rect(const int32_t pos_x, const int32_t pos_y,
           const uint32_t window_width, const uint32_t window_height,
           const uint32_t window_border)
    : xaxis_pos(pos_x), yaxis_pos(pos_y), width(window_width),
      height(window_height), border(window_border) {}

} // namespace slacker