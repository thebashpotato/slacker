#ifndef SLACKER_X_DISPLAY_HPP
#define SLACKER_X_DISPLAY_HPP

#include "slacker/utils/attributes.hpp"
#include "slacker/utils/result.hpp"
#include "slacker/utils/tagged_type.hpp"
#include <X11/Xlib.h>
#include <cstdint>
#include <memory>

namespace slacker::x {

    namespace detail {
        class ScreenIdTag {};
    }// namespace detail

    using ScreenId = utils::TaggedFundamental<detail::ScreenIdTag, int32_t>;

    class X11Display;
    using SharedX11DisplayPtr = std::shared_ptr<X11Display>;

    /**
     * @brief Wraps the most important object in an X program, the Display *
     * */
    class SLACKER_EXPORT X11Display {
    private:
        std::shared_ptr<Display> display_;

    public:
        /**
         * @brief Calls XOpenDisplay, and sets a custom lambda function deleter that
         * calls XCloseDisplay on object destruction.
         *
         * @details Use the static open method instead of using this constructor directly.
         * */
        X11Display();

        /**
         * @brief  Default destructor
         *
         * @details Since a custom lambda deleter is used in the constructor,
         * and the underlying Display is wrapped in a shared_ptr, our memory
         * is guaranteed to be cleaned up.
         **/
        virtual ~X11Display() = default;


        /**
         * @brief Move constructor and assignment
         * */
        X11Display(X11Display &&other) noexcept;
        auto operator=(X11Display &&other) noexcept -> X11Display &;


        /**
         * @brief Disallow copying
         * */
        X11Display(const X11Display &other) = delete;
        auto operator=(const X11Display &other) -> X11Display & = delete;

    public:
        /**
         * @brief Gets a shared instace to X11Display interface
         * */
        [[nodiscard]] static auto open() -> utils::Result<SharedX11DisplayPtr, utils::Void>;


        /**
         * @brief Check if the display opened correctly.
         *
         * @returns false if XOpenDisplay Failed, true for success.
         * */
        [[nodiscard]] auto isOpen() const -> bool;


        /**
         * @brief Gets the underlying raw Display *
         * */
        [[nodiscard]] auto raw() const -> Display *;


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
         * @brief Gets root window based on the current screenId
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
