#include <X11/X.h>
#include <X11/Xlib.h>
#include <slacker/geometry.hpp>
#include <slacker/window.hpp>
#include <utility>

namespace slacker {
    SlackerWindow::SlackerWindow(SlackerXConnPtr slacker_conn_ptr)
        : m_slacker_conn_ptr(std::move(slacker_conn_ptr)) {}

    SlackerWindow::SlackerWindow(SlackerXConnPtr slacker_conn_ptr, int32_t screen)
        : m_slacker_conn_ptr(std::move(slacker_conn_ptr)), m_is_root(true),
          m_window(RootWindow(m_slacker_conn_ptr->get_raw_display(), screen)) {}

    SlackerWindow::~SlackerWindow() {
        this->unmap();
        if (this->m_is_window_alloc && this->m_window != 0) {
            // TODO: Check Return code
            XDestroyWindow(this->m_slacker_conn_ptr->get_raw_display(), this->m_window);
            this->m_window = 0;
        }
    }

    auto SlackerWindow::get_window() const noexcept -> Window {
        return this->m_window;
    }

    auto SlackerWindow::is_root() const noexcept -> bool { return this->m_is_root; }

    auto SlackerWindow::create_window(const SlackerWindow &root_window, Rect &&rect,
                                      int32_t screen) noexcept -> bool {
        if (!root_window.is_root() || this->is_root()) {
            return false;
        }
        XSetWindowAttributes xwa;
        xwa.background_pixel =
                WhitePixel(this->m_slacker_conn_ptr->get_raw_display(), screen);
        xwa.border_pixel =
                BlackPixel(this->m_slacker_conn_ptr->get_raw_display(), screen);
        xwa.event_mask = ButtonPress;

        this->m_window = XCreateWindow(
                this->m_slacker_conn_ptr->get_raw_display(), root_window.get_window(),
                rect.xaxis_pos, rect.yaxis_pos, rect.width, rect.height, rect.border,
                DefaultDepth(this->m_slacker_conn_ptr->get_raw_display(), screen),
                InputOutput,
                DefaultVisual(this->m_slacker_conn_ptr->get_raw_display(), screen),
                CWBackPixel | CWBorderPixel | CWEventMask, &xwa);

        if ((this->m_window == BadAlloc) || (this->m_window == BadMatch) ||
            (this->m_window == BadValue) || (this->m_window == BadWindow)) {
            return false;
        }
        return true;
    }

    auto SlackerWindow::map() noexcept -> bool {
        if (!this->m_is_mapped && !this->m_is_root) {
            if ((XMapWindow(this->m_slacker_conn_ptr->get_raw_display(),
                            this->m_window)) == BadWindow) {
                return false;
            }
            this->m_is_mapped = true;
        }
        return true;
    }

    auto SlackerWindow::unmap() const noexcept -> void {
        if (this->m_is_mapped) {
            XUnmapWindow(this->m_slacker_conn_ptr->get_raw_display(), this->m_window);
        }
    }

}// namespace slacker
