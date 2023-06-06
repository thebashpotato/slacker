#include "slacker/x/display.hpp"
#include <X11/Xlib.h>

namespace slacker::x {
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


    auto X11Display::open() -> etl::Result<SharedX11DisplayPtr, etl::Void> {
        auto display = std::make_shared<X11Display>();
        if (display->isOpen()) {
            return etl::Result<SharedX11DisplayPtr, etl::Void>(std::move(display));
        }
        return etl::Result<SharedX11DisplayPtr, etl::Void>(etl::Void());
    }


    auto X11Display::isOpen() const -> bool {
        return display_ != nullptr;
    }


    auto X11Display::raw() const -> Display * {
        return display_.get();
    }


    auto X11Display::screenId() const -> int32_t {
        return DefaultScreen(display_.get());
    }


    auto X11Display::screenCount() const -> int32_t {
        return XScreenCount(display_.get());
    }


    auto X11Display::defaultRoot() const -> Window {
        return DefaultRootWindow(display_.get());
    }


    auto X11Display::root() const -> Window {
        return RootWindow(display_.get(), screenId());
    }


    auto X11Display::serverVendor() const -> std::string {
        return XServerVendor(display_.get());
    }
}// namespace slacker::x
