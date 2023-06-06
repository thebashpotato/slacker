#include "slacker/x/window.hpp"
#include "slacker/pure/geometry.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <utility>

namespace slacker::x {
    X11Window::X11Window(SharedX11DisplayPtr display) noexcept
        : display_(std::move(display)) {}


    X11Window::~X11Window() {
        unmap();
        if (isAllocated_ && window_ != 0) {
            // TODO: Check Return code
            XDestroyWindow(display_->raw(), window_);
            window_ = 0;
        }
    }


    auto X11Window::getWindow() const noexcept -> Window {
        return window_;
    }


    auto X11Window::createWindow(pure::Rect &&rect) noexcept -> bool {
        XSetWindowAttributes xwa;
        xwa.background_pixel =
                WhitePixel(display_->raw(), display_->screenId());
        xwa.border_pixel =
                BlackPixel(display_->raw(), display_->screenId());
        xwa.event_mask = ButtonPress;

        window_ = XCreateWindow(
                display_->raw(), display_->root(),
                rect.xpos(), rect.ypos(), rect.width(), rect.height(), 0,
                DefaultDepth(display_->raw(), display_->screenId()),
                InputOutput,
                DefaultVisual(display_->raw(), display_->screenId()),
                CWBackPixel | CWBorderPixel | CWEventMask, &xwa);

        // TODO: X return codes need to be wrapped
        return (window_ != BadAlloc) && (window_ != BadMatch) &&
               (window_ != BadValue) && (window_ != BadWindow);
    }


    auto X11Window::map() noexcept -> bool {
        if (!isMapped_) {
            if ((XMapWindow(display_->raw(),
                            window_)) == BadWindow) {
                return false;
            }
            isMapped_ = true;
        }
        return true;
    }


    auto X11Window::unmap() const noexcept -> void {
        if (isMapped_) {
            XUnmapWindow(display_->raw(), window_);
        }
    }

}// namespace slacker::x
