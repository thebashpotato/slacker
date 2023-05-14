#ifndef SLACKER_DISPLAY_HPP
#define SLACKER_DISPLAY_HPP

#include <X11/Xlib.h>
#include <memory>
#include <slacker/attributes.hpp>

namespace slacker {
    using SharedXDisplayPtr = std::shared_ptr<Display>;
    /**
     * @brief Wraps the most important object in an X program, the Display *
     * */
    class SLACKER_EXPORT X11Display {
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
         * @brief Used to construct the class
         * */
        [[nodiscard]] static auto open() -> std::unique_ptr<X11Display>;

        /**
         * @brief Check if the display opened correctly.
         *
         * @returns false if XOpenDisplay Failed, true for success.
         * */
        [[nodiscard]] auto isOpen() const -> bool;

        /**
         * @brief Gets the SharedXDisplayPtr
         * */
        [[nodiscard]] auto shared_display() const -> SharedXDisplayPtr;

        /**
         * @brief Gets the raw Display *
         * */
        [[nodiscard]] auto raw_display() const -> Display *;

        /**
         * @brief Gets the server vendor for the X11 version that is running
         *
         * @details String that provides some identification of the X server implementation.
         * If the data returned by the server is in the Latin Portable Character Encoding,
         * then the string is in the Host Portable Character Encoding.
         * Otherwise, the contents of the string are implementation-dependent.
         * */
        [[nodiscard]] auto serverVendor() const -> std::string;

    private:
        SharedXDisplayPtr display_;
    };
}// namespace slacker

#endif
