#include <X11/X.h>
#include <X11/Xlib.h>
#include <slacker/geometry.hpp>
#include <slacker/window.hpp>
#include <utility>

namespace slacker {
    X11Window::X11Window(SharedXDisplayPtr shared_display_ptr)
        : shared_display_ptr_(std::move(shared_display_ptr)) {}

    X11Window::X11Window(SharedXDisplayPtr shared_display_ptr, int32_t screen)
        : shared_display_ptr_(std::move(shared_display_ptr)), is_root_(true),
          window_(RootWindow(shared_display_ptr_.get(), screen)) {}

    X11Window::~X11Window() {
        this->unmap();
        if (is_window_alloc_ && window_ != 0) {
            // TODO: Check Return code
            XDestroyWindow(shared_display_ptr_.get(), window_);
            window_ = 0;
        }
    }

    auto X11Window::get_window() const noexcept -> Window {
        return window_;
    }

    auto X11Window::is_root() const noexcept -> bool { return this->is_root_; }

    auto X11Window::create_window(const X11Window &root_window, Rect &&rect,
                                  int32_t screen) noexcept -> bool {
        if (!root_window.is_root() || this->is_root()) {
            return false;
        }
        XSetWindowAttributes xwa;
        xwa.background_pixel =
                WhitePixel(shared_display_ptr_.get(), screen);
        xwa.border_pixel =
                BlackPixel(shared_display_ptr_.get(), screen);
        xwa.event_mask = ButtonPress;

        window_ = XCreateWindow(
                shared_display_ptr_.get(), root_window.get_window(),
                rect.xaxis_pos, rect.yaxis_pos, rect.width, rect.height, rect.border,
                DefaultDepth(shared_display_ptr_.get(), screen),
                InputOutput,
                DefaultVisual(shared_display_ptr_.get(), screen),
                CWBackPixel | CWBorderPixel | CWEventMask, &xwa);

        // TODO: X return codes need to be wrapped
        return (window_ != BadAlloc) && (window_ != BadMatch) &&
               (window_ != BadValue) && (window_ != BadWindow);
    }

    auto X11Window::map() noexcept -> bool {
        if (!is_mapped_ && !is_root_) {
            if ((XMapWindow(shared_display_ptr_.get(),
                            window_)) == BadWindow) {
                return false;
            }
            this->is_mapped_ = true;
        }
        return true;
    }

    auto X11Window::unmap() const noexcept -> void {
        if (is_mapped_) {
            XUnmapWindow(shared_display_ptr_.get(), this->window_);
        }
    }

}// namespace slacker
