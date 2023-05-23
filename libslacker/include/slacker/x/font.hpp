#ifndef SLACKER_X_FONT_HPP
#define SLACKER_X_FONT_HPP

#include "slacker/pure/geometry.hpp"
#include "slacker/x/display.hpp"
#include <X11/Xft/Xft.h>
#include <fontconfig/fontconfig.h>

namespace slacker::x {
    /**
     * @brief Wrapper around XftFont and FcPattern
     * */
    class X11Font {
    private:
        slacker::x::SharedX11DisplayPtr display_{nullptr};
        std::string fontname_{};
        FcPattern *pattern_{nullptr};
        slacker::pure::Height height_{0};
        XftFont *xfont_{nullptr};

    public:
        /**
         * @brief Constructs the default monospace font with size of 12
         * */
        explicit X11Font(slacker::x::SharedX11DisplayPtr &display);


        /**
         * @brief Constructs a custom font based of a fontname
         * */
        X11Font(slacker::x::SharedX11DisplayPtr &display, std::string fontname);


        /**
         * @brief Constructs a custom font based of a font pattern
         * */
        X11Font(slacker::x::SharedX11DisplayPtr &display, FcPattern *pattern);


        /**
         * @brief If this font has an allocated pattern or xfont, XftFontClose,
         * and FcPatternDestroy calls are made freeing the memory for this font.
         * */
        ~X11Font();


        /**
         * @brief default copy and move constructors
         * */
        X11Font(const X11Font &other) = default;
        auto operator=(const X11Font &other) -> X11Font & = default;
        X11Font(X11Font &&other) = default;
        auto operator=(X11Font &&other) -> X11Font & = default;

    public:
        /**
         * @brief default copy and move constructors
         * */
        [[nodiscard]] auto fontname() const noexcept -> std::string;


        /**
         * @brief default copy and move constructors
         * */
        [[nodiscard]] auto pattern() const noexcept -> FcPattern *;


        /**
         * @brief default copy and move constructors
         * */
        [[nodiscard]] auto height() const noexcept -> slacker::pure::Height;


        /**
         * @brief default copy and move constructors
         * */
        [[nodiscard]] auto xfont() const noexcept -> XftFont *;
    };

}// namespace slacker::x

#endif
