/// MIT License
///
/// Copyright (c) 2023 Matt Williams
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.

#ifndef ETL_HPP
#define ETL_HPP

#if __cplusplus >= 201702L

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>

namespace etl {
    /**
     * @brief Ditch those old C style for loops and iterate over your enums safely with ranged for loops.
     *
     * @detail Currently assumes the enumeration is contiguous (no gaps).
     *
     * @example tests/enum_iterable_test.cpp
     * */
    template<typename EnumIterable, EnumIterable beginValue, EnumIterable endValue>
    class EnumerationIterator {
    private:
        // Verifys the type is indeed an enumeration class
        // https://en.cppreference.com/w/cpp/types/underlying_type
        using value_t = typename std::underlying_type<EnumIterable>::type;
        std::int64_t value_;

    public:
        /**
         * @brief Default constructor builds an instance to the first value
         * in the enumeration.
         *
         * @detail Used in the begin() method.
         * */
        EnumerationIterator() noexcept : value_(static_cast<value_t>(beginValue)) {}


        /**
         * @brief Constructs an instance to a specified value.
         *
         * @detail Used in the end() method.
         * */
        explicit EnumerationIterator(const EnumIterable &iter) noexcept
            : value_(static_cast<value_t>(iter)) {}


        /**
         * @brief Default Destructor Move/Copy constructor and assignment
         * */
        virtual ~EnumerationIterator() = default;
        EnumerationIterator(EnumerationIterator &&other) noexcept = default;
        auto operator=(EnumerationIterator &&other) noexcept -> EnumerationIterator & = default;
        EnumerationIterator(EnumerationIterator const &other) = default;
        auto operator=(EnumerationIterator const &other) -> EnumerationIterator & = default;

    public:
        /**
         * @brief ++this overload
         *
         * @detail Increments the underlying value and then returns
         * an instance of itself. this++ not implemented, as it is
         * ineffecient and usually not needed.
         * */
        [[maybe_unused]] auto operator++() noexcept -> EnumerationIterator {
            ++this->value_;
            return *this;
        }


        /**
         * @brief Dereference overload
         *
         * @detail Gets an instance to the current underlying value
         * after casting it the type EnumIterable.
         * */
        [[nodiscard]] auto operator*() noexcept -> EnumIterable {
            return static_cast<EnumIterable>(value_);
        }


        /**
         * @brief Is equal overload
         * */
        [[nodiscard]] auto
        operator==(EnumerationIterator const &other_iterator) const noexcept -> bool {
            return value_ == other_iterator.value_;
        }


        /**
         * @brief Not equal overload
         * */
        [[nodiscard]] auto
        operator!=(EnumerationIterator const &other_iterator) const noexcept -> bool {
            return !(*this == other_iterator);
        }

    public:
        /**
         * @brief Return the beginning value, this will use the default constructor.
         * */
        [[nodiscard]] auto begin() const noexcept -> EnumerationIterator {
            return *this;
        }


        /**
         * @brief Return the end value.
         * */
        [[nodiscard]] auto end() noexcept -> EnumerationIterator {
            // cache the value
            static const auto endIter = ++EnumerationIterator(endValue);
            return endIter;
        }
    };


    /**
     * @brief Tag a primitive fundamental type to descriptive class names.
     *
     * @detail Solves the issue when a function takes in many arguments of the same type,
     * which can lead to confusion and mistakes. For example consider a Rectangle with a constructor
     * that takes a uint32_t width, and uint32_t height. The width and the height can easily be swapped,
     * resulting in runtime bugs. This can be avoided by "Tagging the type".
     *
     * @example tests/tagged_type_test.cpp
     * */
    template<typename Tag, typename FundamentalType>
    class TaggedFundamental {
    private:
        FundamentalType value_{};

    public:
        /**
         * @brief All the constructors needed to build a fundamental wrapped type
         * */
        TaggedFundamental() {
            static_assert(std::is_fundamental<FundamentalType>::value);
        };


        explicit TaggedFundamental(FundamentalType const &value) : value_(value) {
            static_assert(std::is_fundamental<FundamentalType>::value);
        }


        explicit TaggedFundamental(FundamentalType &&value) : value_(value) {
            static_assert(std::is_fundamental<FundamentalType>::value);
        }


        /**
         * @brief Default Destructor Move/Copy constructor and assignment
         * */
        virtual ~TaggedFundamental() = default;
        TaggedFundamental(TaggedFundamental &&other) noexcept = default;
        auto operator=(TaggedFundamental &&other) noexcept -> TaggedFundamental & = default;
        TaggedFundamental(TaggedFundamental const &other) = default;
        auto operator=(TaggedFundamental const &other) -> TaggedFundamental & = default;

    public:
        /**
         * @brief Get the underlying type as a const ref
         * */
        [[nodiscard]] inline auto get() const noexcept -> FundamentalType const & { return value_; }


        /**
         * @brief Get the underlying type as immutable
         * */
        [[nodiscard]] inline auto getReadOnly() const noexcept -> FundamentalType { return value_; }
    };


    /**
     * @brief Holds useful runtime source code location information for use in Errors.
     *
     * @detail Should not be used directly, rather the user should pass the `etl::RUNTIME_INFO`
     * macro invocation.
     * */
    class SourceLocation {
    private:
        std::string file_{};
        uint32_t line_{};
        std::string func_{};

    public:
        SourceLocation() = delete;

        /**
         * @brief The only valid constructor
         *
         * @param `file` name of the file where the error occurred
         * @param `line` line number where the error occurred
         * @param `func` name of the function where the error occurred
         * */
        SourceLocation(std::string_view const &file, uint32_t line,
                       std::string_view const &func) noexcept
            : file_(file), line_(line), func_(func) {}


        /**
         * @brief Default Destructor, Move/Copy constructor and assignment
         * */
        virtual ~SourceLocation() = default;
        SourceLocation(SourceLocation &&other) noexcept = default;
        auto operator=(SourceLocation &&other) noexcept -> SourceLocation & = default;
        SourceLocation(SourceLocation const &other) = default;
        auto operator=(SourceLocation const &other) -> SourceLocation & = default;

    public:
        /**
         * @brief Get the file name in which the error occured
         * */
        [[nodiscard]] inline auto file() const noexcept -> std::string_view {
            return file_;
        }


        /**
         * @brief Get the line number in which the error occured
         * */
        [[nodiscard]] inline auto line() const noexcept -> uint32_t {
            return line_;
        }

        /**
         * @brief Get the name of the function in which the error occured
         * */
        [[nodiscard]] inline auto function() const noexcept -> std::string_view {
            return func_;
        }
    };


/**
 * @brief Wrapper macro which constructs an instance of SourceLocation in-place
 * and guarantees that the accurate file, function name, and line number will be
 * reported.
 * */
#ifdef __linux__
#define RUNTIME_INFO                   \
    SourceLocation(__FILE__, __LINE__, \
                   static_cast<const char *>(__PRETTY_FUNCTION__))
#else
#define RUNTIME_INFO \
    SourceLocation(__FILE__, __LINE__, static_cast<const char *>(__func__))
#endif


    /**
     * @brief Interface for an Err class
     * */
    class IError {
    public:
        IError() = default;
        virtual ~IError() = default;
        IError(IError &&other) noexcept = default;
        auto operator=(IError &&other) noexcept -> IError & = default;
        IError(IError const &other) = default;
        auto operator=(IError const &other) -> IError & = default;

    public:
        [[nodiscard]] virtual inline auto msg() const noexcept -> std::string = 0;
        [[nodiscard]] virtual inline auto info() const noexcept -> std::string = 0;
    };


    /**
     * @brief A basic Error object which can be built using an error message and the
     * SourceLocation RUNTIME_INFO macro.
     * */
    class Error : public IError {
    private:
        std::string msg_{};
        std::string info_{};

    private:
        /**
         * @brief Constructs the error with only a message
         * */
        explicit Error(std::string_view const &msg) noexcept : msg_(msg) {}


        /**
         * @brief Constructs the error with the message and source location
         * using move semantics
         * */
        Error(std::string_view const &msg, SourceLocation &&slc) noexcept : msg_(msg) {
            info_.append("Error: ")
                    .append(msg)
                    .append("\nFunction: ")
                    .append(slc.function())
                    .append("\nFile: ")
                    .append(slc.file())
                    .append(":")
                    .append(std::to_string(slc.line()));
        }

    public:
        /**
         * @brief Default Destructor, Move/Copy constructor and assignment
         * */
        ~Error() override = default;
        Error(Error &&other) noexcept = default;
        auto operator=(Error &&other) noexcept -> Error & = default;
        Error(Error const &other) = default;
        auto operator=(Error const &other) -> Error & = default;

    public:
        /**
         * @brief Creates an Error object with only an error message via string_view
         * */
        [[nodiscard]] inline static auto create(std::string_view const &msg) -> Error {
            auto error = Error(msg);
            return error;
        }


        /**
         * @brief Creates an Error object with error message and source location information
         * using move semantics
         * */
        [[nodiscard]] inline static auto create(std::string_view const &msg, SourceLocation &&slc) -> Error {
            auto error = Error(msg, std::move(slc));
            return error;
        }

    public:
        /**
         * @brief Get just the error message
         * */
        [[nodiscard]] inline auto msg() const noexcept -> std::string override {
            return msg_;
        }


        /**
         * @brief Override the current error message, useful when using the Result.mapErr method.
         * */
        [[nodiscard]] inline auto set(std::string_view &&msg) noexcept {
            msg_ = msg;
        }


        /**
         * @brief Override the current error message, useful when using the Result.mapErr method.
         * */
        [[nodiscard]] inline auto set(std::string_view const &msg) noexcept {
            msg_ = msg;
        }


        /**
         * @brief Get the pre-formatted (pretty printed) error string.
         *
         * @details If Error was not created with the RUNTIME_INFO macro info_ will be empty,
         * in which case the msg_ will be returned instead.
         * */
        [[nodiscard]] inline auto info() const noexcept -> std::string override {
            if (!info_.empty()) {
                return info_;
            }
            return msg_;
        }
    };


    /**
     * @brief Empty stub type for when the user wants a result with an Ok type
     * with no value, since c++ doesn't have rusts () type, this is my workaround.
     *
     * @detail In Rust it would be Result<(), String>,
     * Converted into this c++ class would be,
     * Result<Void, std::string>;
     * */
    class Void {};


    /**
     * @brief Generic Result type modeled after the Rust lanaguage's Result<T, E>
     * */
    template<typename OkType, typename ErrType>
    class Result {
    private:
        std::variant<OkType, ErrType> result_;
        bool isOk_;

    public:
        /**
         * @brief All the constructors needed to build an OkType or ErrType
         * */
        explicit Result(OkType const &value) : result_(value), isOk_(true) {}
        explicit Result(OkType &&value) : result_(std::move(value)), isOk_(true) {}
        explicit Result(ErrType const &error) : result_(error), isOk_(false) {}
        explicit Result(ErrType &&error) : result_(std::move(error)), isOk_(false) {}


        /**
         * @brief Default Destructor, Move/Copy constructor and assignment
         * */
        virtual ~Result() = default;
        Result(Result &&other) noexcept = default;
        auto operator=(Result &&other) noexcept -> Result & = default;
        Result(const Result &other) = default;
        auto operator=(const Result &other) -> Result & = default;

    public:
        /**
         * @brief Check if the union value is of the ok type
         * */
        [[nodiscard]] inline auto isOk() const noexcept -> bool {
            return isOk_;
        }


        /**
         * @brief Check if the union value is of the error type
         * */
        [[nodiscard]] inline auto isErr() const noexcept -> bool {
            return !isOk_;
        }


        /**
         * @brief Check if the union value is of the error type
         *
         * @detail The use should always use isOk() before using ok()
         *
         * @return std::optinal<OkType> for safety, incase the user did not call
         * isOk() before using this method.
         * */
        [[nodiscard]] inline auto ok() const noexcept -> std::optional<OkType> {
            if (isOk_) {
                if (auto *value = std::get_if<OkType>(&result_)) {
                    return *value;
                }
            }
            return std::nullopt;
        }


        /**
         * @brief Check if the union value is of the error type
         *
         * @detail The use should always use isErr() before using err()
         *
         * @return std::optinal<ErrType> for safety, incase the user did not call
         * isErr() before using this method.
         * */
        [[nodiscard]] inline auto err() const noexcept -> std::optional<ErrType> {
            if (!isOk_) {
                if (auto *err = std::get_if<ErrType>(&result_)) {
                    return *err;
                }
            }
            return std::nullopt;
        }


        /**
         * @brief Maps a custom/lambda function to the OkType leaving the ErrType untouched.
         *
         * @detail The use should always use isOk() before using map()
         *
         * @return Returns the Result type with a modified OkType, or just the Result unmodified
         * if isOK() is false.
         * */
        template<typename Function>
        [[nodiscard]] inline auto map(Function &&func) const noexcept -> Result<OkType, ErrType> {
            if (isOk_) {
                if constexpr (std::is_invocable_r_v<OkType, Function, OkType const &>) {
                    return Result<OkType, ErrType>(std::invoke(std::forward<Function>(func), *std::get_if<OkType>(&result_)));
                } else {
                    return Result<OkType, ErrType>(*std::get_if<OkType>(&result_));
                }
            }
            return Result<OkType, ErrType>(std::move(*std::get_if<ErrType>(&result_)));
        }


        /**
         * @brief Maps a custom/lambda function to the ErrType leaving the OkType untouched.
         *
         * @detail The use should always use the isErr() method before using mapErr().
         *
         * @return Returns the Result type with a modified ErrType, or just the Result unmodified
         * if isErr() is false.
         * */
        template<typename Function>
        [[nodiscard]] inline auto mapErr(Function &&func) const noexcept -> Result<OkType, ErrType> {
            if (!isOk_) {
                if constexpr (std::is_invocable_r_v<ErrType, Function, ErrType const &>) {
                    return Result<OkType, ErrType>(std::invoke(std::forward<Function>(func), *std::get_if<ErrType>(&result_)));
                } else {
                    return Result<OkType, ErrType>(*std::get_if<ErrType>(&result_));
                }
            }
            return Result<OkType, ErrType>(std::move(*std::get_if<OkType>(&result_)));
        }
    };


    /**
     * @brief Result Template Specialization for std::unique_ptr.
     *
     * @detail Since unique_ptr is a move only type, the Generic Result implementation
     * confuses which constructors are to be used and its not enough. Objects with deleted copy constructors
     * will need a templated specialization. You can use this specialiation for your custom type as
     * an example on how to do it.
     * */
    template<typename OkType, typename ErrType>
    class Result<std::unique_ptr<OkType>, ErrType> {
    private:
        std::variant<std::unique_ptr<OkType>, ErrType> result_;
        bool isOk_;

    public:
        explicit Result(std::unique_ptr<OkType> &&value) : result_(std::move(value)), isOk_(true) {}
        explicit Result(const ErrType &error) : result_(error), isOk_(false) {}
        explicit Result(ErrType &&error) : result_(std::move(error)), isOk_(false) {}

    public:
        /**
         * @brief Check if the union value is of the ok type
         * */
        [[nodiscard]] inline auto isOk() const noexcept -> bool {
            return isOk_;
        }


        /**
         * @brief Check if the union value is of the error type
         * */
        [[nodiscard]] inline auto isErr() const noexcept -> bool {
            return !isOk_;
        }


        /**
         * @brief Check if the union value is of the error type
         *
         * @detail The use should always use isOk() before using ok()
         *
         * @return std::optinal<OkType> for safety, incase the user did not call
         * isOk() before using this method.
         * */
        [[nodiscard]] inline auto ok() const noexcept -> std::optional<std::unique_ptr<OkType>> {
            if (isOk_) {
                if (auto *value = std::get_if<std::unique_ptr<OkType>>(&result_)) {
                    return std::make_unique<OkType>(**value);
                }
            }
            return std::nullopt;
        }


        /**
         * @brief Check if the union value is of the error type
         *
         * @detail The use should always use isErr() before using err()
         *
         * @return std::optinal<ErrType> for safety, incase the user did not call
         * isErr() before using this method.
         * */
        [[nodiscard]] inline auto err() const noexcept -> std::optional<ErrType> {
            if (!isOk_) {
                if (auto *err = std::get_if<ErrType>(&result_)) {
                    return *err;
                }
            }
            return std::nullopt;
        }
    };

}// namespace etl

#endif// __cplusplus >= 201702l

#endif// ETL_HPP
