#include <X11/Xlib.h>
#include <slacker/display.hpp>

namespace slacker {
    X11Display::X11Display()
        : display_(
                  XOpenDisplay(nullptr),
                  [](Display *display) { XCloseDisplay(display); }) {}


    X11Display::X11Display(X11Display &&other) noexcept : display_(std::move(other.display_)) {}

    auto X11Display::operator=(X11Display &&other) noexcept -> X11Display & {
        if (this != &other) {
            display_ = std::move(other.display_);
        }
        return *this;
    }

    auto X11Display::open() -> std::unique_ptr<X11Display> {
        auto display = std::make_unique<X11Display>();
        if (!display->isOpen()) {
            return nullptr;
        }
        return display;
    }

    auto X11Display::isOpen() const -> bool {
        return display_ != nullptr;
    }

    auto X11Display::shared_display() const -> SharedXDisplayPtr {
        return display_;
    }

    auto X11Display::raw_display() const -> Display * {
        return display_.get();
    }

    auto X11Display::screen_id() const -> int32_t {
        return DefaultScreen(display_.get());
    }

    auto X11Display::screen_count() const -> int32_t {
        return XScreenCount(display_.get());
    }

    auto X11Display::default_root() const -> Window {
        return DefaultRootWindow(display_.get());
    }

    auto X11Display::root() const -> Window {
        return RootWindow(display_.get(), screen_id());
    }

    auto X11Display::serverVendor() const -> std::string {
        return XServerVendor(display_.get());
    }
}// namespace slacker
