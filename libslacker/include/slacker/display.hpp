#ifndef SLACKER_DISPLAY_HPP_
#define SLACKER_DISPLAY_HPP_

#include <X11/Xlib.h>
#include <memory>
#include <slacker/attributes.hpp>
#include <string_view>

namespace slacker {
    /**
 * @brief Wraps the most important object in an X program, the Display *
 *
 * @detail Handles memory management of Display*, the `SlackerDisplay` class
 * should be passed around to other objects like `SlackerWindow` as a
 * shared_ptr, using the `SlackerXConnPtr` alias at the bottom of this file.
 * Since the Display is essentially just a connection to the X server, it needs
 * to be shared, naturally in C++ a reference counted object is the best way to
 * do this as the overhead of counting references (incrementing/decrementing an
 * integer) is negligible at best.
 * */
    class SLACKER_EXPORT SlackerDisplay {
    public:
        /**
   * @brief Calls XOpenDisplay with the name of the display.
   * If the XOpenDisplay function fails, m_display_failure will be set to false
   * as it is not safe to throw exceptions in constructors. The boolean should
   * be checked after creating the display with the init_failure() method.
   *
   * @param `name` id of the display, can be left empty for NULL
   * */
        explicit SlackerDisplay(std::string_view &&name = "");

        /**
   * @brief  Calls XCloseDisplay if the underlying dpy pointer
   * is not nullptr.
   * */
        virtual ~SlackerDisplay();

        /**
   * @brief Default copy/move constructors and assignment operators
   * */
        SlackerDisplay(const SlackerDisplay &other) = default;
        SlackerDisplay &operator=(const SlackerDisplay &other) = default;
        SlackerDisplay(SlackerDisplay &&other) = default;
        SlackerDisplay &operator=(SlackerDisplay &&other) = default;

    public:
        /**
   * @brief Gets the underlying raw display pointer
   * */
        [[nodiscard]] auto get_raw_display() -> Display *;

        /**
   * @brief Signals if the XOpenDisplay failed
   * */
        [[nodiscard]] auto init_failure() const noexcept -> bool;

    private:
        Display *m_display{nullptr};
        bool m_init_failure{false};
    };

    /**
 * @brief Alias for the SlackerDisplay class as shared_ptr.
 *
 * @details It is called SlackerXConnPtr because the underlying Display *
 * is really just a connection to the X server. Since the connection must be
 * shared between many objects in X programs, it is safer to pass our wrapper
 * context around as reference counted pointer.
 * */
    using SlackerXConnPtr = std::shared_ptr<SlackerDisplay>;
}// namespace slacker

#endif
