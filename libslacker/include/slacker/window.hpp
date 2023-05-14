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
    class SLACKER_EXPORT X11Window {
    public:
        /**
         * @brief Default constructor
         * */
        X11Window() = default;

        /**
         * @brief Creates a root window.
         *
         * @param `shared_display_ptr` a reference counted connection to the X server.
         * @param `screen` number of screens on the host
         * */
        X11Window(SharedXDisplayPtr shared_display_ptr, int32_t screen);

        /**
         * @brief Creates a child window
         *
         * @param `shared_display_ptr` a reference counted connection to the X server.
         * */
        explicit X11Window(SharedXDisplayPtr shared_display_ptr);

        /**
         * @brief Unmaps a window if it is mapped, and destroys resources
         * if there were any allocated for this window.
         * */
        virtual ~X11Window();

        /**
         * @brief Default copy/move constructors and assignment operators
         * */
        X11Window(const X11Window &other) = default;
        auto operator=(const X11Window &other) -> X11Window & = default;
        X11Window(X11Window &&other) = default;
        auto operator=(X11Window &&other) -> X11Window & = default;

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
        [[nodiscard]] auto create_window(const X11Window &root_window,
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
        SharedXDisplayPtr shared_display_ptr_{nullptr};
        bool is_mapped_{false};
        bool is_root_{false};
        bool is_window_alloc_{false};
        Window window_{0};
    };
}// namespace slacker

#endif
