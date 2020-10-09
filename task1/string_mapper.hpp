#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <optional>
#include <memory>

using std::operator""sv;

template<class From, auto target>
struct Mapping;

template<class Base, class Target, class... Mappings>
struct ClassMapper {
    static std::optional<Target> map(const Base& object);
};

template<class Base, class Target>
struct ClassMapper<Base, Target> {
    static std::optional<Target> map(const Base& object) {
        return std::nullopt;
    }
};

template<class Base, class Target, Target target, class DynamicType, class... Mappings>
struct ClassMapper<Base, Target, Mapping<DynamicType, target>, Mappings...> {
    static std::optional<Target> map(const Base& object) {
        if (typeid(object) == typeid(std::declval<DynamicType>())) {
            return std::make_optional(target);
        } else {
            return ClassMapper<Base, Target, Mappings...>::map(object);
        }
    }
};

template<size_t max_length>
struct String {
    constexpr String(const char* string, size_t length)
    : length(length) {
        for (size_t i = 0; i < length; ++i) {
            this->string[i] = string[i];
        }
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

// static_assert(std::is_literal_type_v<String<256>>);
// static_assert(std::is_same_v<String<256>, decltype("smth"_cstr)>);
// static_assert("some text"_cstr == "some text"sv);
// static_assert(String<128>{"some text", 4} == "some"sv);
//
// class Animal {
// public:
//     virtual ~Animal() = default;
// };
//
// class Cat : public Animal {};
// class Cow : public Animal {};
// class Dog : public Animal {};
// class StBernard : public Dog {};
// class Horse : public Animal {};
// class RaceHorse : public Horse {};
