#include <X11/Xlib.h>
#include <slacker/display.hpp>
#include <string_view>

namespace slacker {
    SlackerDisplay::SlackerDisplay(std::string_view &&name) : m_display(XOpenDisplay(name.data())) {
        if (this->m_display == nullptr) {
            this->m_init_failure = true;
        }
    }

    SlackerDisplay::~SlackerDisplay() {
        if (this->m_display != nullptr) {
            XCloseDisplay(this->m_display);
            this->m_display = nullptr;
        }
    }

    auto SlackerDisplay::get_raw_display() -> Display * { return this->m_display; }

    auto SlackerDisplay::init_failure() const noexcept -> bool {
        return this->m_init_failure;
    }
}// namespace slacker
