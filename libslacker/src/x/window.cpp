#include "slacker/x/window.hpp"
#include "slacker/pure/geometry.hpp"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <utility>

namespace slacker::x {
    X11Window::X11Window(SharedXDisplayPtr sharedDpyPtr) noexcept
        : sharedDpyPtr_(std::move(sharedDpyPtr)) {}


    X11Window::~X11Window() {
        this->unmap();
        if (isAllocated_ && window_ != 0) {
            // TODO: Check Return code
            XDestroyWindow(sharedDpyPtr_.get(), window_);
            window_ = 0;
        }
    }


    auto X11Window::getWindow() const noexcept -> Window {
        return window_;
    }


    auto X11Window::createWindow(Window root_window, int32_t screen, pure::Rect &&rect) noexcept -> bool {
        XSetWindowAttributes xwa;
        xwa.background_pixel =
                WhitePixel(sharedDpyPtr_.get(), screen);
        xwa.border_pixel =
                BlackPixel(sharedDpyPtr_.get(), screen);
        xwa.event_mask = ButtonPress;

        window_ = XCreateWindow(
                sharedDpyPtr_.get(), root_window,
                rect.xaxis_pos, rect.yaxis_pos, rect.width, rect.height, rect.border,
                DefaultDepth(sharedDpyPtr_.get(), screen),
                InputOutput,
                DefaultVisual(sharedDpyPtr_.get(), screen),
                CWBackPixel | CWBorderPixel | CWEventMask, &xwa);

        // TODO: X return codes need to be wrapped
        return (window_ != BadAlloc) && (window_ != BadMatch) &&
               (window_ != BadValue) && (window_ != BadWindow);
    }


    auto X11Window::map() noexcept -> bool {
        if (!isMapped_) {
            if ((XMapWindow(sharedDpyPtr_.get(),
                            window_)) == BadWindow) {
                return false;
            }
            this->isMapped_ = true;
        }
        return true;
    }


    auto X11Window::unmap() const noexcept -> void {
        if (isMapped_) {
            XUnmapWindow(sharedDpyPtr_.get(), this->window_);
        }
    }

}// namespace slacker::x
