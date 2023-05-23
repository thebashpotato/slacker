#include "slacker/x/font.hpp"
#include <X11/Xft/Xft.h>

namespace slacker::x {
    constexpr auto DEFAULT_FONT_NAME = "monospace:size=12";
    constexpr auto DEFAULT_FONT_HEIGHT = 12;

    X11Font::X11Font(slacker::x::SharedX11DisplayPtr &display)
        : display_(std::move(display)),
          fontname_(DEFAULT_FONT_NAME),
          height_(DEFAULT_FONT_HEIGHT) {
    }


    X11Font::X11Font(slacker::x::SharedX11DisplayPtr &display, std::string fontname)
        : display_(std::move(display)),
          fontname_(std::move(fontname)) {}


    X11Font::X11Font(slacker::x::SharedX11DisplayPtr &display, FcPattern *pattern)
        : display_(std::move(display)),
          pattern_(pattern) {}


    X11Font::~X11Font() {
        if (xfont_ != nullptr) {
            //XftFontClose(display_->raw(), xfont_);
        }
        if (pattern_ != nullptr) {
            FcPatternDestroy(pattern_);
        }
    }


    auto X11Font::fontname() const noexcept -> std::string {
        return fontname_;
    }


    auto X11Font::pattern() const noexcept -> FcPattern * {
        return pattern_;
    }


    auto X11Font::height() const noexcept -> slacker::pure::Height {
        return height_;
    }


    auto X11Font::xfont() const noexcept -> XftFont * {
        return xfont_;
    }

}// namespace slacker::x
