#ifndef SLACKER_WINDOW_HPP
#define SLACKER_WINDOW_HPP

#include <X11/Xlib.h>
#include <cstdint>
#include <memory>
#include <slacker/attributes.hpp>
#include <slacker/display.hpp>

namespace slacker {
    class Rect;

    /**
     * @brief Wrapper around an X Window instance
     * */
    class SLACKER_EXPORT SlackerWindow {
    public:
        /**
         * @brief Default constructor
         * */
        SlackerWindow() = default;

        /**
         * @brief Creates a root window.
         *
         * @param `slacker_conn_ptr` a reference counted connection to the X server.
         * @param `screen` number of screens on the host
         * */
        SlackerWindow(SlackerXConnPtr slacker_conn_ptr, int32_t screen);

        /**
         * @brief Creates a child window
         *
         * @param `slacker_conn_ptr` a reference counted connection to the X server.
         * */
        explicit SlackerWindow(SlackerXConnPtr slacker_conn_ptr);

        /**
         * @brief Unmaps a window if it is mapped, and destroys resources
         * if there were any allocated for this window.
         * */
        virtual ~SlackerWindow();

        /**
         * @brief Default copy/move constructors and assignment operators
         * */
        SlackerWindow(const SlackerWindow &other) = default;
        auto operator=(const SlackerWindow &other) -> SlackerWindow & = default;
        SlackerWindow(SlackerWindow &&other) = default;
        auto operator=(SlackerWindow &&other) -> SlackerWindow & = default;

    public:
        /**
         * @brief Getter for underlying window
         *
         * @returns a copy of the window id
         * */
        [[nodiscard]] auto get_window() const noexcept -> Window;

        /**
         * @brief Is this window the root window.
         * */
        [[nodiscard]] auto is_root() const noexcept -> bool;

    public:
        /**
         * @brief Wrapper around XCreateWindow
         *
         * @param `root_window` The root window instance from which this window will
         * be a child of.
         * @param `rect` Window geometry object
         * @param `screen` number of screens for the host
         *
         * @error XCreateSimpleWindow can generate BadAlloc, BadMatch, BadValue, and
         * BadWindow errors.
         * */
        [[nodiscard]] auto create_window(const SlackerWindow &root_window,
                                         Rect &&rect, int32_t screen) noexcept
                -> bool;
        /**
         * @brief Wrapper around XUnmapWindow.
         *
         * @detail Only maps the window iff it is not already mapped, and if it is not
         * the root window.
         *
         * @returns false if BadWindow error code is received from XMapWindow,
         * true if Success
         * */
        [[nodiscard]] auto map() noexcept -> bool;

        /**
         * @brief Wrapper around XUnmapWindow
         *
         * @detail Does not free any memory e.g. XDestroyWindow is not called,
         * that happens in the Destructor
         * */
        auto unmap() const noexcept -> void;

    private:
        SlackerXConnPtr m_slacker_conn_ptr{nullptr};
        bool m_is_mapped{false};
        bool m_is_root{false};
        bool m_is_window_alloc{false};
        Window m_window{0};
    };

}// namespace slacker

#endif
