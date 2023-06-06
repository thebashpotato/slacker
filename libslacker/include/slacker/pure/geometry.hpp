#ifndef SLACKER_GEOMETRY_HPP
#define SLACKER_GEOMETRY_HPP

#include "slacker/utils/attributes.hpp"
#include "slacker/vendor/etl.hpp"
#include <cstdint>
#include <stack>

namespace slacker::pure {
    /**
     * @brief the detail namespace denotes private types and variables that are
     * not included in the public Api
     * */
    namespace detail {
        /**
         * @brief Default constant values for default Rect
         * */
        constexpr auto XAXIS_POSITION = 500;
        constexpr auto YAXIS_POSITION = 500;
        constexpr auto WIDTH = 500;
        constexpr auto HEIGHT = 500;

        /**
         * @brief Tagged types for Rect parameters
         * */
        class XpositionTag {};
        class YpositionTag {};
        class WidthTag {};
        class HeightTag {};
    }// namespace detail

    using Xposition = etl::TaggedFundamental<detail::XpositionTag, int32_t>;
    using Yposition = etl::TaggedFundamental<detail::YpositionTag, int32_t>;
    using Width = etl::TaggedFundamental<detail::WidthTag, uint32_t>;
    using Height = etl::TaggedFundamental<detail::HeightTag, uint32_t>;


    /**
     * @brief Describes the position, height, width and border of a window.
     * */
    class SLACKER_EXPORT Rect {
    public:
        Rect() noexcept : xpos_(detail::XAXIS_POSITION), ypos_(detail::YAXIS_POSITION), width_(detail::WIDTH), height_(detail::HEIGHT) {}

        Rect(Xposition xpos, Yposition ypos, Width width, Height height) noexcept;

    public:
        /**
         * @brief Gets the underlying primitive type
         * */
        [[nodiscard]] auto xpos() const noexcept -> int32_t;

        /**
         * @brief Gets the underlying primitive type
         * */
        [[nodiscard]] auto ypos() const noexcept -> int32_t;

        /**
         * @brief Gets the underlying primitive type
         * */
        [[nodiscard]] auto width() const noexcept -> uint32_t;

        /**
         * @brief Gets the underlying primitive type
         * */
        [[nodiscard]] auto height() const noexcept -> uint32_t;

    private:
        Xposition xpos_;
        Yposition ypos_;
        Width width_;
        Height height_;
    };
}// namespace slacker::pure

#endif// SLACKER_GEOMETRY_HPP
