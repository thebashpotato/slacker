#ifndef SLACKER_UTILS_RESULT_HPP
#define SLACKER_UTILS_RESULT_HPP

#include "slacker/utils/attributes.hpp"
#include <functional>
#include <optional>
#include <variant>

namespace slacker::utils {

    /**
     * @brief Empty stub type for when the user wants a result with an Ok type
     * with no value, since c++ doesn't have rusts () type, this is my workaround.
     *
     * In Rust it would be Result<(), String>,
     * Converted into this c++ class would be,
     * Result<Void, std::string>;
     * */
    class SLACKER_EXPORT Void {};

    /**
     * @brief Generic Result type modeled after the Rust lanaguage's Result<T, E>
     * */
    template<typename T, typename E>
    class SLACKER_EXPORT Result {
    private:
        std::variant<T, E> result_;
        bool isOk_;

    public:
        explicit Result(const T &value) : result_(value), isOk_(true) {}
        explicit Result(T &&value) : result_(std::move(value)), isOk_(true) {}
        explicit Result(const E &error) : result_(error), isOk_(false) {}
        explicit Result(E &&error) : result_(std::move(error)), isOk_(false) {}


        [[nodiscard]] inline auto isOk() const -> bool {
            return isOk_;
        }


        [[nodiscard]] inline auto isErr() const -> bool {
            return !isOk_;
        }


        [[nodiscard]] inline auto ok() const -> std::optional<T> {
            if (isOk_) {
                return std::get<T>(result_);
            }
            return std::nullopt;
        }


        [[nodiscard]] inline auto err() const -> std::optional<E> {
            if (!isOk_) {
                return std::get<E>(result_);
            }
            return std::nullopt;
        }


        template<typename F>
        [[nodiscard]] inline auto map(F &&func) const -> Result<T, E> {
            if (isOk_) {
                return Result<T, E>(std::invoke(std::forward<F>(func), std::get<T>(result_)));
            }
            return Result<T, E>(std::get<E>(result_));
        }


        template<typename F>
        [[nodiscard]] inline auto mapErr(F &&func) const -> Result<T, E> {
            if (!isOk_) {
                return Result<T, E>(std::invoke(std::forward<F>(func), std::get<E>(result_)));
            }
            return Result<T, E>(std::get<T>(result_));
        }
    };
}// namespace slacker::utils

#endif// SLACKER_UTILS_RESULT_HPP
