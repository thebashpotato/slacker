#include "slacker_wm/draw.hpp"
#include "slacker/x/display.hpp"


namespace slacker_wm {
    Draw::Draw(slacker::x::SharedX11DisplayPtr display,
               slacker::x::X11Font font,
               slacker::pure::Width const &width,
               slacker::pure::Height const &height)
        : display_(std::move(display)),
          font_(std::move(font)),
          width_(width),
          height_(height),
          drawable_(XCreatePixmap(display_->raw(), display_->root(), width_.get(), height_.get(), static_cast<uint32_t>(DefaultDepth(display_->raw(), display_->screenId())))),
          graphics_context_(XCreateGC(display_->raw(), display_->root(), 0, nullptr)),
          colorsheme_() {
        XSetLineAttributes(display_->raw(), graphics_context_, 1, LineSolid, CapButt, JoinMiter);
    }


    Draw::~Draw() {
        if (drawable_ != 0) {
            XFreePixmap(display_->raw(), drawable_);
        }
        XFreeGC(display_->raw(), graphics_context_);
    }


    auto Draw::resize(slacker::pure::Width const &width, slacker::pure::Height const &height) -> void {
        width_ = width;
        height_ = height;
        if (drawable_ != 0) {
            XFreePixmap(display_->raw(), drawable_);
        }
        drawable_ = XCreatePixmap(display_->raw(), display_->root(), width_.get(), height_.get(), static_cast<uint32_t>(DefaultDepth(display_.get(), display_->screenId())));
    }

}// namespace slacker_wm
