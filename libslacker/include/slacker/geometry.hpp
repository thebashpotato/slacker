#ifndef SLACKER_GEOMETRY_HPP
#define SLACKER_GEOMETRY_HPP
#include <cstdint>
#include <slacker/attributes.hpp>

namespace slacker {

/**
 * @brief Describes the position, height, width and border of a window.
 * */
class SLACKER_EXPORT Rect {
public:
  Rect();

  Rect(int32_t pos_x, int32_t pos_y, uint32_t width, uint32_t height,
       uint32_t border);

public:
  int32_t xaxis_pos{};
  int32_t yaxis_pos{};
  uint32_t width{};
  uint32_t height{};
  uint32_t border{};
};
} // namespace slacker
#endif // SLACKER_GEOMETRY_HPP
