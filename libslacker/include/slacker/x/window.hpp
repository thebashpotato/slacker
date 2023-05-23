#ifndef SLACKER_WINDOW_HPP
#define SLACKER_WINDOW_HPP

#include "slacker/utils/attributes.hpp"
#include "slacker/x/display.hpp"
#include <X11/Xlib.h>
#include <cstdint>
#include <memory>

namespace slacker::pure {
    class Rect;
}

namespace slacker::x {

    /**
     * @brief Wrapper around an X Window instance
     * */
    class SLACKER_EXPORT X11Window {
    private:
        SharedX11DisplayPtr display_{nullptr};
        bool isMapped_{false};
        bool isAllocated_{false};
        Window window_{0};

    public:
        /**
         * @brief Default constructor
         * */
        X11Window() noexcept = default;


        /**
         * @brief Creates a child window
         *
         * @param `sharedDpyPtr` a reference counted connection to the X server.
         * */
        explicit X11Window(SharedX11DisplayPtr display) noexcept;


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
        [[nodiscard]] auto getWindow() const noexcept -> Window;


        /**
         * @brief Wrapper around XCreateWindow
         *
         * @param `rect` Window geometry object
         *
         * @error XCreateSimpleWindow can generate BadAlloc, BadMatch, BadValue, and
         * BadWindow errors.
         * */
        [[nodiscard]] auto createWindow(pure::Rect &&rect) noexcept -> bool;


        /**
         * @brief Wrapper around XUnmapWindow.
         *
         * @detail Only maps the window iff it is not already mapped, and if it is not
         * the screen_id window.
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
    };
}// namespace slacker::x

#endif
