#include <iostream>
#include <type_traits>
#include <optional>
#include <string>
#include <array>
#include <tuple>
#include <utility>
#include <cstdint>
#include <limits>

template <class Enum, size_t MAXN = 512>
    requires std::is_enum_v<Enum>
class EnumeratorTraits {
public:
    static constexpr size_t size() noexcept {
        return std::get<2>(info);
    }

    static constexpr Enum at(size_t i) noexcept {
        return std::get<0>(info)[i];
    }

    static constexpr std::string_view nameAt(size_t i) noexcept {
        return std::get<1>(info)[i];
    }

private:
    using T = std::underlying_type_t<Enum>;

    static constexpr int64_t getMinValue() {
        int64_t low = static_cast<int64_t>(std::numeric_limits<T>::min());
        if (low < 0) {
            low = std::max(low, -static_cast<int64_t>(MAXN));
        }
        return low;
    }

    static constexpr int64_t getMaxValue() {
        uint64_t high = static_cast<uint64_t>(std::numeric_limits<T>::max());
        return static_cast<int64_t>(std::min(high, MAXN));
    }

    static constexpr int64_t min_value_ = getMinValue();
    static constexpr int64_t max_value_ = getMaxValue();

    template<Enum... enums>
    static constexpr auto init3() {
        std::array<Enum, max_value_ - min_value_ + 1> values;
        std::array<std::string_view, max_value_ - min_value_ + 1> names;
        size_t count = 0;

        auto signature = static_cast<std::string_view>(__PRETTY_FUNCTION__);
        signature.remove_prefix(signature.find_first_of('{') + 1);
        signature.remove_suffix(signature.size() - signature.find_first_of('}'));

        for (size_t i = 0; i <= MAXN * 2; ++i) {
            if (signature[0] != '(') {
                size_t colon_pos = signature.find_first_of(':');
                if (colon_pos != signature.npos) {
                    signature.remove_prefix(colon_pos + 2);
                }
                values[count] = static_cast<Enum>(min_value_ + i);
                names[count] = signature.substr(0, signature.find_first_of(','));
                ++count;
            }
            size_t next_space_pos = signature.find_first_of(' ');
            if (next_space_pos == signature.npos) {
                break;
            } else {
                signature.remove_prefix(next_space_pos + 1);
            }
        }
        return std::make_tuple(values, names, count);
    }

    template<int64_t... ints>
    static constexpr auto init2(std::integer_sequence<int64_t, ints...>) {
        return init3<static_cast<Enum>(ints + min_value_)...>();
    }

    static constexpr auto init() {
        return init2(std::make_integer_sequence<int64_t, max_value_ - min_value_ + 1>());
    }

    static constexpr auto info = init();
};
