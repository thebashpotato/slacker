#ifndef SLACKER_UTILS_TAGGED_TYPE_HPP
#define SLACKER_UTILS_TAGGED_TYPE_HPP

#include "slacker/utils/attributes.hpp"
#include <cstdint>
#include <type_traits>

namespace slacker::utils {
    /**
     * @brief Tag primitive fundamental types to descriptive class names.
     * */
    template<typename Tag, typename FundamentalType>
    class SLACKER_EXPORT TaggedFundamental {
    private:
        FundamentalType value_{};

    public:
        TaggedFundamental() {
            static_assert(std::is_fundamental<FundamentalType>::value);
        };

        explicit TaggedFundamental(FundamentalType const &value) : value_(value) {
            static_assert(std::is_fundamental<FundamentalType>::value);
        }

    public:
        auto get() const -> FundamentalType const & { return value_; }

        auto getReadOnly() const -> FundamentalType { return value_; }
    };
}// namespace slacker::utils

#endif
