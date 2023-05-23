#include "slacker/pure/geometry.hpp"
#include <cstdint>

namespace slacker::pure {

    Rect::Rect(Xposition xpos, Yposition ypos, Width width, Height height) noexcept
        : xpos_(xpos), ypos_(ypos), width_(width), height_(height) {}


    auto Rect::xpos() const noexcept -> int32_t {
        return xpos_.get();
    }


    auto Rect::ypos() const noexcept -> int32_t {
        return ypos_.get();
    }


    auto Rect::width() const noexcept -> uint32_t {
        return width_.get();
    }


    auto Rect::height() const noexcept -> uint32_t {
        return height_.get();
    }

}// namespace slacker::pure
