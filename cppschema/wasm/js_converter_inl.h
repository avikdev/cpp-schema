#pragma once

#include <utility>
#include <type_traits>

#include "absl/log/log.h"
#include "cppschema/common/enum_registry.h"  // IWYU pragma: keep
#include "cppschema/common/strong_types.h"  // IWYU pragma: keep
#include "cppschema/common/types.h"
#include "cppschema/common/visitor_macros.h"  // IWYU pragma: keep

namespace cppschema::jsbridge {

// WARNING: Do not include this header directly, it is meant to be included only by js_converter.h
// It contains template specializations, and is separated to avoid cluttering the main header.
//
// This could also be enforced by creating a decicated cc_library rule for this header with private
// visibility, but for simplicity we just rely on the convention.

// Forward re-declaration to suppress IDE warnings.
template <typename T, typename Enable>
struct JSConverter;

//-----------------------------------------------------------------------------
// Type Detection Traits
//-----------------------------------------------------------------------------
namespace internal {

// Primitive-like: Smaller types with direct emscripten support.
template <typename T>
struct is_primitive_like : std::disjunction<
    std::is_same<T, bool>,
    std::is_same<T, std::string>,
    std::is_same<T, float>,
    std::is_same<T, int32_t>,
    std::is_same<T, uint32_t>,
    std::is_same<T, int64_t>,
    std::is_same<T, uint64_t>
> {};

// VOID: Type trait / Concept to identify VoidType (represents void in C++).
template <typename T> struct is_void_like : std::false_type {};
template <> struct is_void_like<VoidType> : std::true_type {};

// TODO: Use concept, like:
// template <typename T> concept is_void_type = std::is_same_v<std::decay_t<T>, VoidType>;

// PAIRS: std::pair.
template <typename T>
struct is_pair_like : std::false_type {};
template <typename T1, typename T2>
struct is_pair_like<std::pair<T1, T2>> : std::true_type {};

// TUPLE: std::tuple.
template <typename T>
struct is_tuple_like : std::false_type {};
template <typename... Ts>
struct is_tuple_like<std::tuple<Ts...>> : std::true_type {};

// MAP: Has key_type and mapped_type
template <typename T, typename = void>
struct is_map_like_impl : std::false_type {};

template <typename T>
struct is_map_like_impl<T, std::void_t<
    typename T::key_type,
    typename T::mapped_type,
    typename T::value_type,
    decltype(std::declval<T&>()[std::declval<typename T::key_type>()])
>> : std::true_type {};

template <typename T>
using is_map_like = is_map_like_impl<T>;

// ARRAY: Has value_type, but NOT a map, and NOT a string
template <typename T, typename = void>
struct is_array_like : std::false_type {};
template <typename T, typename A> struct is_array_like<std::vector<T, A>> : std::true_type {};

// SET: Has key_type and value_type, but they are the same
template <typename T, typename = void>
struct is_set_like : std::false_type {};
template <typename T>
struct is_set_like<T, std::void_t<typename T::key_type, typename T::value_type>> 
    : std::bool_constant<std::is_same_v<typename T::key_type, typename T::value_type> && !is_array_like<T>::value> {};

// OPTIONAL: std::optional.
template <typename T> struct is_optional_like : std::false_type {};
template <typename U> struct is_optional_like<std::optional<U>> : std::true_type {};

// Visible struct like, i.e. a struct whose members are visible. Supports only those C++ structs
// which has defined the visitor MACRO.

// Mock visitor used only for the concept check to ensure _visit_members exists and is callable.
struct ProbeVisitor {
    template<typename T> void visitfn(const char*, T&) {}
};

template<typename T, typename = void>
struct is_visible_struct_like : std::false_type {};

template<typename T>
struct is_visible_struct_like<
    T,
    std::void_t<
        decltype(
            std::declval<T&>()._visit_members(
                std::declval<ProbeVisitor&>()
            )
        )
    >
> : std::true_type {};

// ENUMS: should be scoped enum (i.e. not trivially convertible to int).
template <typename T>
struct is_enum_like : std::bool_constant<
    std::is_enum_v<T> &&
    !std::is_convertible_v<T, int>
> {};

// STRONG TYPES: Types that are wrappers around primitives, but are not implicitly convertible
// to them.
template<typename T>
struct is_strong_type_like : std::false_type {};
template <typename T, typename Tag>
struct is_strong_type_like<StrongType<T, Tag>> : std::true_type {};

// Unsupported type: One which satisfies none of the above.
template<typename T>
struct is_unsupported_like : std::conjunction<
    std::negation<is_primitive_like<T>>,
    std::negation<is_void_like<T>>,
    std::negation<is_pair_like<T>>,
    std::negation<is_tuple_like<T>>,
    std::negation<is_array_like<T>>,
    std::negation<is_map_like<T>>,
    std::negation<is_set_like<T>>,
    std::negation<is_optional_like<T>>,
    std::negation<is_visible_struct_like<T>>,
    std::negation<is_enum_like<T>>,
    std::negation<is_strong_type_like<T>>
> {};

}  // namespace internal

//-----------------------------------------------------------------------------
// Specialization Implementations
//-----------------------------------------------------------------------------

// PRIMITIVES: string, boolean, int32_t etc.
template <typename PrimitiveType>
struct JSConverter<PrimitiveType, std::enable_if_t<internal::is_primitive_like<PrimitiveType>::value>> {
    // Default: Fallback to Embind's internal conversion for primitives
    static emscripten::val toJS(const PrimitiveType& value) {
        return emscripten::val(value);
    }
    static PrimitiveType fromJS(emscripten::val v) {
        return v.as<PrimitiveType>();
    }
};

// VOID: VoidType (represents void in C++).
template <typename VoidLikeType>
struct JSConverter<VoidLikeType, std::enable_if_t<internal::is_void_like<VoidLikeType>::value>> {
    static emscripten::val toJS(const VoidLikeType& value) {
        return emscripten::val::object();
    }
    static VoidLikeType fromJS(emscripten::val v) {
        return {};
    }
};

// PAIRS: std::pair
template <typename PairType>
struct JSConverter<PairType, std::enable_if_t<internal::is_pair_like<PairType>::value>> {
    static emscripten::val toJS(const PairType& value) {
        emscripten::val arr = emscripten::val::array();
        arr.call<void>("push", JSConverter<typename PairType::first_type>::toJS(value.first));
        arr.call<void>("push", JSConverter<typename PairType::second_type>::toJS(value.second));
        return arr;
    }

    static PairType fromJS(emscripten::val v) {
        PairType value;
        unsigned int len = v["length"].as<unsigned int>();
        if (len != 2) {
            return {};
        }
        value.first = JSConverter<typename PairType::first_type>::fromJS(v[0]);
        value.second = JSConverter<typename PairType::second_type>::fromJS(v[1]);
        return value;
    }
};

// TUPLES: std::tuple
template <typename TupleType>
struct JSConverter<TupleType, std::enable_if_t<internal::is_tuple_like<TupleType>::value>> {
    static emscripten::val toJS(const TupleType& value) {
        emscripten::val arr = emscripten::val::array();
        std::apply([&arr](const auto&... elems) {
            auto visit = [&arr]<typename T>(const T& x) {
                arr.call<void>("push", JSConverter<T>::toJS(x));
            };
            (visit(elems), ...);
        }, value);
        return arr;
    }

    static TupleType fromJS(emscripten::val v) {
        TupleType tpl;
        std::apply([&v](auto&... elems) {
            size_t index = 0;
            auto visit = [&index, &v]<typename T>(T& x) {
                x = JSConverter<T>::fromJS(v[index++]);
            };
            (visit(elems), ...);
        }, tpl);
        return tpl;
    }
};

// MAPS: std::map, flat_map, etc.
template <typename MapType>
struct JSConverter<MapType, std::enable_if_t<internal::is_map_like<MapType>::value>> {
    static emscripten::val toJS(const MapType& m) {
        emscripten::val obj = emscripten::val::object();
        for (const auto& [key, value] : m) {
            obj.set(key, JSConverter<typename MapType::mapped_type>::toJS(value));
        }
        return obj;
    }

    static MapType fromJS(emscripten::val v) {
        MapType m;
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", v);
        const size_t len = keys["length"].as<size_t>();
        for (size_t i = 0; i < len; ++i) {
            emscripten::val k = keys[i];
            m[k.as<typename MapType::key_type>()] = 
                JSConverter<typename MapType::mapped_type>::fromJS(v[k]);
        }
        return m;
    }
};

// ARRAYS: std::vector, std::list, std::deque
template <typename ArrayType>
struct JSConverter<ArrayType, std::enable_if_t<internal::is_array_like<ArrayType>::value>> {
    static emscripten::val toJS(const ArrayType& container) {
        emscripten::val arr = emscripten::val::array();
        for (const auto& item : container) {
            arr.call<void>("push", JSConverter<typename ArrayType::value_type>::toJS(item));
        }
        return arr;
    }

    static ArrayType fromJS(emscripten::val v) {
        ArrayType container;
        unsigned int len = v["length"].as<unsigned int>();
        for (unsigned int i = 0; i < len; ++i) {
            // Use back_inserter if available, or just push_back for vector/list
            container.push_back(JSConverter<typename ArrayType::value_type>::fromJS(v[i]));
        }
        return container;
    }
};

// SETS: std::set, flat_set
template <typename SetType>
struct JSConverter<SetType, std::enable_if_t<internal::is_set_like<SetType>::value>> {
    static emscripten::val toJS(const SetType& s) {
        emscripten::val arr = emscripten::val::array();
        for (const auto& item : s) {
            arr.call<void>("push", JSConverter<typename SetType::value_type>::toJS(item));
        }
        return arr;
    }

    static SetType fromJS(emscripten::val v) {
        SetType s;
        unsigned int len = v["length"].as<unsigned int>();
        for (unsigned int i = 0; i < len; ++i) {
            s.insert(JSConverter<typename SetType::value_type>::fromJS(v[i]));
        }
        return s;
    }
};

// OPTIONAL: std::optional
template <typename OptionalType>
struct JSConverter<OptionalType, std::enable_if_t<internal::is_optional_like<OptionalType>::value>> {
    static emscripten::val toJS(const OptionalType& opt) {
        if (opt.has_value()) {
            return JSConverter<typename OptionalType::value_type>::toJS(opt.value());
        } else {
            return emscripten::val::undefined();
        }
    }
    static OptionalType fromJS(emscripten::val v) {
        if (v.isNull() || v.isUndefined()) {
            return std::nullopt;
        } else {
            return JSConverter<typename OptionalType::value_type>::fromJS(v);
        }
    }
};

// Visible (visitable) STRUCTS.
template <typename StructType>
struct JSConverter<StructType, std::enable_if_t<internal::is_visible_struct_like<StructType>::value>> {
    static emscripten::val toJS(const StructType& s) {
        emscripten::val obj = emscripten::val::object();
        auto lambda = [&obj]<typename T>(const char* name, const T& t) -> void {
            obj.set(name, JSConverter<T>::toJS(t));
        };
        s._visit_members(lambda);
        return obj;
    }

    static StructType fromJS(emscripten::val v) {
        StructType s;
        auto lambda = [&v]<typename T>(const char* name, T& t) -> void {
            if (v.hasOwnProperty(name)) {
                t = JSConverter<T>::fromJS(v[name]);
            } else {
                t = T{};  // Default initialize the member if the property is missing in the JS object.
            }
        };
        s._visit_members(lambda);
        return s;
    }
};

// ENUM:
template <typename EnumType>
struct JSConverter<EnumType, std::enable_if_t<internal::is_enum_like<EnumType>::value>> {
    using ToEnumFunc = EnumRegistry::ToEnumFunc<EnumType>;
    using ToInfoFunc = EnumRegistry::ToInfoFunc<EnumType>;

    static emscripten::val toJS(const EnumType& value) {
        const ToInfoFunc toInfo = EnumRegistry::instance().getToInfo<EnumType>();
        if (!toInfo) {
            LOG(FATAL) << "Enum not registered";
        }
        const auto [name, ordinal] = toInfo(value);
        return emscripten::val(std::string(name));
    }

    static EnumType fromJS(emscripten::val v) {
        const ToEnumFunc toEnum = EnumRegistry::instance().getToEnum<EnumType>();
        if (!toEnum) {
            LOG(FATAL) << "Enum not registered";
        }
        const std::string strval = v.as<std::string>();
        std::optional<EnumType> enumv = toEnum(v.as<std::string>());
        if (enumv.has_value()) {
            return std::move(enumv).value();
        }
        return EnumType{};
    }
};

// STRONG TYPES: In JS it's same as the underlying primitive type.
template <typename StrongType>
struct JSConverter<StrongType, std::enable_if_t<internal::is_strong_type_like<StrongType>::value>> {
    static emscripten::val toJS(const StrongType& s) {
        return JSConverter<typename StrongType::value_type>::toJS(s.value());
    }

    static StrongType fromJS(emscripten::val v) {
        return StrongType(JSConverter<typename StrongType::value_type>::fromJS(v));
    }
};

// Invisible (non-visitable) STRUCTS. This is implemented just to give a better error feedback.
template <typename FallbackType>
struct JSConverter<FallbackType, std::enable_if_t<internal::is_unsupported_like<FallbackType>::value>> {
    static emscripten::val toJS(const FallbackType& s) {
        static_assert(false, "unsupported type");
    }

    static FallbackType fromJS(emscripten::val v) {
        static_assert(false, "unsupported type");
    }
};

}  // namespace cppschema::jsbridge
