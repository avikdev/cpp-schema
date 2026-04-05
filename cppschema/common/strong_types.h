#pragma once

#include <string>  // IWYU pragma: keep

template <typename T, typename Tag>
struct StrongType {
    using value_type = T;
    static constexpr bool is_strong_type_v = true; // Detection marker

    T value = T();

    // Explicit constructor prevents accidental implicit conversions
    explicit constexpr StrongType(T val) : value(val) {}
    constexpr StrongType() : value{} {}

    // Comparison operators (C++20 spaceship operator)
    auto operator<=>(const StrongType&) const = default;
    bool operator==(const StrongType&) const = default;

    // Conversion back to the primitive (explicit for safety)
    explicit constexpr operator T() const { return value; }
};

// General macro for any underlying type
#define DEFINE_STRONG_TYPE(Name, Primitive) \
    struct Name##Tag {}; \
    using Name = StrongType<Primitive, Name##Tag>;

// Specific shortcut macros
#define DEFINE_STRONG_INT_TYPE(Name) DEFINE_STRONG_TYPE(Name, int32_t)
#define DEFINE_STRONG_UINT_TYPE(Name) DEFINE_STRONG_TYPE(Name, uint32_t)
#define DEFINE_STRONG_FLOAT_TYPE(Name) DEFINE_STRONG_TYPE(Name, float)
#define DEFINE_STRONG_STRING_TYPE(Name) DEFINE_STRONG_TYPE(Name, std::string)
