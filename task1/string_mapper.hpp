#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <optional>
#include <memory>
#include <typeinfo>

using std::operator""sv;

template<class From, auto target>
struct Mapping;

template<class Base, class Target, class... Mappings>
struct ClassMapper {
    static std::optional<Target> map(const Base& object);
};

template<class Base, class Target>
struct ClassMapper<Base, Target> {
    static std::optional<Target> map(const Base&) {
        return std::nullopt;
    }
};

template<class Base, class Target, Target target, class From, class... Mappings>
    requires std::is_base_of_v<Base, From>
struct ClassMapper<Base, Target, Mapping<From, target>, Mappings...> {
    static std::optional<Target> map(const Base& object) {
        try {
            dynamic_cast<const From&>(object);
            return std::make_optional(target);
        } catch(const std::bad_cast& e) {
            return ClassMapper<Base, Target, Mappings...>::map(object);
        }
    }
};

template<size_t max_length>
struct String {
    constexpr String(const char* string, size_t length)
    : length(length) {
        std::copy(string, string + length, this->string);
    }

    constexpr operator std::string_view () const {
        return std::string_view(string, length);
    }

    char string[max_length];
    size_t length;
};

constexpr String<256> operator "" _cstr(const char* string, size_t length) {
    return String<256>(string, length);
}
