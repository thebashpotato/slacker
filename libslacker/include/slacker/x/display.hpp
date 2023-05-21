#ifndef SLACKER_X_DISPLAY_HPP
#define SLACKER_X_DISPLAY_HPP

#include "slacker/utils/attributes.hpp"
#include <X11/Xlib.h>
#include <memory>

namespace slacker::x {
    using SharedXDisplayPtr = std::shared_ptr<Display>;
    /**
     * @brief Wraps the most important object in an X program, the Display *
     * */
    class SLACKER_EXPORT X11Display {
    private:
        SharedXDisplayPtr display_;

    public:
        /**
         * @brief Calls XOpenDisplay, and sets a custom lambda function deleter that
         * calls XCloseDisplay on object destruction.
         *
         * @details Use the static open method instead of using this constructor directly.
         * */
        X11Display();

        /**
         * @brief default move constructor and assignment
         * */
        X11Display(X11Display &&other) noexcept;
        auto operator=(X11Display &&other) noexcept -> X11Display &;


        /**
         * @brief  Default destructor
         *
         * @details Since a custom lambda deleter is used in the constructor,
         * and the underlying Display is wrapped in a shared_ptr, our memory
         * is guaranteed to be cleaned up.
         **/
        virtual ~X11Display() = default;

        /**
         * @brief Disallow copying
         * */
        X11Display(const X11Display &other) = delete;
        auto operator=(const X11Display &other) -> X11Display & = delete;

    public:
        /**
         * @brief Check if the display opened correctly.
         *
         * @returns false if XOpenDisplay Failed, true for success.
         * */
        [[nodiscard]] auto isOpen() const -> bool;


        /**
         * @brief Gets the SharedXDisplayPtr
         * */
        [[nodiscard]] auto sharedDisplay() const -> SharedXDisplayPtr;


        /**
         * @brief Gets the raw Display *
         * */
        [[nodiscard]] auto rawDisplay() const -> Display *;


        /**
         * @brief Gets active screen_id.
         * */
        [[nodiscard]] auto screenId() const -> int32_t;


        /**
         * @brief Gets number of screens
         * */
        [[nodiscard]] auto screenCount() const -> int32_t;


        /**
         * @brief Gets default root window
         * */
        [[nodiscard]] auto defaultRoot() const -> Window;


        /**
         * @brief Gets root window based on screen_id
         * */
        [[nodiscard]] auto root() const -> Window;


        /**
         * @brief Gets the server vendor for the X11 version that is running
         *
         * @details String that provides some identification of the X server implementation.
         * If the data returned by the server is in the Latin Portable Character Encoding,
         * then the string is in the Host Portable Character Encoding.
         * Otherwise, the contents of the string are implementation-dependent.
         * */
        [[nodiscard]] auto serverVendor() const -> std::string;
    };
}// namespace slacker::x

#endif// SLACKER_X_DISPLAY_HPP
