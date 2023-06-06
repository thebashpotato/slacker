#ifndef SLACKER_WM_DRAW_HPP
#define SLACKER_WM_DRAW_HPP

#include <X11/X.h>
#include <cstdint>
#include <memory>
#include <slacker.hpp>
#include <vector>


namespace slacker_wm {

    class X11Cursor {
    private:
        Cursor cursor_;

    public:
        X11Cursor() = default;
    };


    class FontManager {
    private:
        std::vector<slacker::x::X11Font> fonts_{};

    public:
        FontManager() = default;
        ~FontManager();

        FontManager(const FontManager &other) = default;
        auto operator=(const FontManager &other) -> FontManager & = default;
        FontManager(FontManager &&other) = default;
        auto operator=(FontManager &&other) -> FontManager & = default;

    public:
        [[nodiscard]] auto loadFont(std::string const &fontname) noexcept -> bool;
    };


    class ColorScheme {
    private:
        XftColor color_;

    public:
        ColorScheme() = default;
        enum {
            Forground,
            Background,
            Border,
        };

    public:
    };


    /**
     * @brief A Drawable abstraction
     **/
    class Draw {
    private:
        slacker::x::SharedX11DisplayPtr display_;
        slacker::x::X11Font font_;
        slacker::pure::Width width_;
        slacker::pure::Height height_;
        Drawable drawable_;
        GC graphics_context_;
        ColorScheme colorsheme_;

    public:
        Draw(slacker::x::SharedX11DisplayPtr display,
             slacker::x::X11Font font,
             slacker::pure::Width const &width,
             slacker::pure::Height const &height);

        ~Draw();

        /**
         * @brief Default copy/move constructors and assignment operators
         * */
        Draw(const Draw &other) = default;
        auto operator=(const Draw &other) -> Draw & = default;
        Draw(Draw &&other) = default;
        auto operator=(Draw &&other) -> Draw & = default;

    public:
        /**
         * @brief Resizes the drawable surface with a new width and height
         * */
        auto resize(slacker::pure::Width const &width, slacker::pure::Height const &height) -> void;
    };


}// namespace slacker_wm

#endif
