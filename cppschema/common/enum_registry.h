#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <typeindex>
#include <optional>
#include <any>
#include <iostream>
#include <utility>

#include "absl/log/log.h"
#include "cppschema/common/visitor_macros.h"

/**
 * @brief A global registry for enum conversion functions. It provides
 * conversion functions to convert from string to enum value, and get the
 * string and ordinal of an enum value.
 *
 * This is used in the library to convert between C++ enums and JS strings in
 * a bidirectional way. In JS the enums are represented as strings, and in C++
 * they are enums.
 * This works only for scoped enums (i.e. those declared with "enum class ...").
 * Example usage:
 *
 * @example
 * enum class NodeTypeEnum { UNKNOWN, INPUT, OUTPUT, FUNCTION };
 * DEFINE_ENUM_CONVERSION_FUNCTION(NodeTypeEnum, UNKNOWN, INPUT, OUTPUT, FUNCTION);
 *
 * This will register the conversion functions for NodeTypeEnum, and you can then use
 * - EnumRegistry::instance().getToEnum<NodeTypeEnum>()
 * - EnumRegistry::instance().getToInfo<NodeTypeEnum>()
 * to get the conversion functions. See the unit tests for example usage.
 */
class EnumRegistry {
public:
    // Signature types for clarity
    template <typename E>
    using ToEnumFunc = std::function<std::optional<E>(const std::string&)>;
    
    template <typename E>
    using ToInfoFunc = std::function<std::pair<std::string, int>(const E)>;

    static EnumRegistry& instance();

    template <typename EnumType, typename F1, typename F2>
    bool registerEnum(F1&& toEnum, F2&& toInfo, const std::string& location) {
        auto tIndex = std::type_index(typeid(EnumType));

        if (locationMap_.count(tIndex)) {
            if (locationMap_[tIndex] == location) {
                return false; // Already registered from this exact spot
            }
            std::cerr << "[EnumRegistry Error] Type conflict for " << typeid(EnumType).name()
                      << "\n  New registration: " << location
                      << "\n  Existing registration: " << locationMap_[tIndex] << std::endl;
            return false;
        }

        // Store type-erased versions of both functions
        toEnumRegistry_[tIndex] = std::any(std::forward<F1>(toEnum));
        toInfoRegistry_[tIndex] = std::any(std::forward<F2>(toInfo));
        locationMap_[tIndex] = location;
        
        return true;
    }

    template <typename EnumType>
    const ToEnumFunc<EnumType> getToEnum() const {
        auto it = toEnumRegistry_.find(std::type_index(typeid(EnumType)));
        if (it != toEnumRegistry_.end()) {
            return std::any_cast<ToEnumFunc<EnumType>>(it->second);
        }
        LOG(FATAL) << "Not found";
        return nullptr;
    }

    template <typename EnumType>
    const ToInfoFunc<EnumType> getToInfo() const {
        auto it = toInfoRegistry_.find(std::type_index(typeid(EnumType)));
        if (it != toInfoRegistry_.end()) {
            return std::any_cast<ToInfoFunc<EnumType>>(it->second);
        }
        LOG(FATAL) << "Not found";
        return nullptr;
    }

private:
    EnumRegistry() = default;
    std::unordered_map<std::type_index, std::any> toEnumRegistry_;
    std::unordered_map<std::type_index, std::any> toInfoRegistry_;
    std::unordered_map<std::type_index, std::string> locationMap_;
};

// Macro Helpers
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define CONCAT_INNER(a, b) a##b
#define CONCAT(a, b) CONCAT_INNER(a, b)

/**
 * @brief Registers bidirectional conversion for an enum.
 * @param EnumType The type of the enum.
 * @param ToEnum Expression: (const std::string&) -> std::optional<EnumType>
 * @param ToInfo Expression: (const EnumType) -> std::pair<std::string, int>
 */

#define ENUM_VALUE_TO_INFO_SINGLE_SWITCH_CASE(field) \
    case ThisEnum::field: return {#field, ordinal};

#define ENUM_NAME_TO_VALUE_SINGLE_DICT_ENTRY(field) \
    {#field, ThisEnum::field},

/**
 * Macro: DEFINE_ENUM_CONVERSION_FUNCTION
 *
 * @brief Defines conversion function from the enum values to string, and from string back to
 * the enum value.
 * 
 * This is used to convert between C++ enum and JS string, as in JS side we use string for
 * an enum type (matches the typescript interfaces).
 * 
 * @example
 * enum class NodeTypeEnum { UNKNOWN, GRAPH_INPUT, GRAPH_OUTPUT, FUNCTION };
 *
 * DEFINE_ENUM_CONVERSION_FUNCTION(NodeTypeEnum, UNKNOWN, GRAPH_INPUT, GRAPH_OUTPUT, FUNCTION);
 *
 * @param EnumType The type of the enum.
 * @param ... Explicit list of the enum values.
 */
#define DEFINE_ENUM_CONVERSION_FUNCTION(EnumType, ...) \
    __attribute__((unused)) static inline const bool __ ## EnumType ## _register ## __LINE__ = []{ \
        using ThisEnum = EnumType; \
        static_assert(!std::is_convertible_v<ThisEnum, int>, "Only scoped enums are supported"); \
        auto toEnum = [](const std::string& name) -> std::optional<ThisEnum> { \
            static const auto* const lookup_table = ([]{ \
                auto* entries_map = new std::unordered_map<std::string, ThisEnum>({ \
                    FOR_EACH(ENUM_NAME_TO_VALUE_SINGLE_DICT_ENTRY, __VA_ARGS__) \
                }); \
                return entries_map; \
            })(); \
            if (auto iter = lookup_table->find(name); iter != lookup_table->end()) { \
                return iter->second; \
            } \
            return std::nullopt; \
        }; \
        auto toInfo = [](const ThisEnum e) -> std::pair<std::string, int> { \
            const int ordinal = static_cast<int>(e); \
            switch (e) { \
                FOR_EACH(ENUM_VALUE_TO_INFO_SINGLE_SWITCH_CASE, __VA_ARGS__) \
            } \
        }; \
        return EnumRegistry::instance().registerEnum<ThisEnum>( \
            EnumRegistry::ToEnumFunc<ThisEnum>(std::move(toEnum)), \
            EnumRegistry::ToInfoFunc<ThisEnum>(std::move(toInfo)), \
            (__FILE__ ":" STR(__LINE__)) \
        ); \
    }();
