#pragma once

// Recursive Expansions. Supports upto 30 members.
#define FE_1(WHAT, X) WHAT(X)
#define FE_2(WHAT, X, ...) WHAT(X) FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X) FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X) FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X) FE_4(WHAT, __VA_ARGS__)
#define FE_6(WHAT, X, ...) WHAT(X) FE_5(WHAT, __VA_ARGS__)
#define FE_7(WHAT, X, ...) WHAT(X) FE_6(WHAT, __VA_ARGS__)
#define FE_8(WHAT, X, ...) WHAT(X) FE_7(WHAT, __VA_ARGS__)
#define FE_9(WHAT, X, ...) WHAT(X) FE_8(WHAT, __VA_ARGS__)
#define FE_10(WHAT, X, ...) WHAT(X) FE_9(WHAT, __VA_ARGS__)
#define FE_11(WHAT, X, ...) WHAT(X) FE_10(WHAT, __VA_ARGS__)
#define FE_12(WHAT, X, ...) WHAT(X) FE_11(WHAT, __VA_ARGS__)
#define FE_13(WHAT, X, ...) WHAT(X) FE_12(WHAT, __VA_ARGS__)
#define FE_14(WHAT, X, ...) WHAT(X) FE_13(WHAT, __VA_ARGS__)
#define FE_15(WHAT, X, ...) WHAT(X) FE_14(WHAT, __VA_ARGS__)
#define FE_16(WHAT, X, ...) WHAT(X) FE_15(WHAT, __VA_ARGS__)
#define FE_17(WHAT, X, ...) WHAT(X) FE_16(WHAT, __VA_ARGS__)
#define FE_18(WHAT, X, ...) WHAT(X) FE_17(WHAT, __VA_ARGS__)
#define FE_19(WHAT, X, ...) WHAT(X) FE_18(WHAT, __VA_ARGS__)
#define FE_20(WHAT, X, ...) WHAT(X) FE_19(WHAT, __VA_ARGS__)
#define FE_21(WHAT, X, ...) WHAT(X) FE_20(WHAT, __VA_ARGS__)
#define FE_22(WHAT, X, ...) WHAT(X) FE_21(WHAT, __VA_ARGS__)
#define FE_23(WHAT, X, ...) WHAT(X) FE_22(WHAT, __VA_ARGS__)
#define FE_24(WHAT, X, ...) WHAT(X) FE_23(WHAT, __VA_ARGS__)
#define FE_25(WHAT, X, ...) WHAT(X) FE_24(WHAT, __VA_ARGS__)
#define FE_26(WHAT, X, ...) WHAT(X) FE_25(WHAT, __VA_ARGS__)
#define FE_27(WHAT, X, ...) WHAT(X) FE_26(WHAT, __VA_ARGS__)
#define FE_28(WHAT, X, ...) WHAT(X) FE_27(WHAT, __VA_ARGS__)
#define FE_29(WHAT, X, ...) WHAT(X) FE_28(WHAT, __VA_ARGS__)
#define FE_30(WHAT, X, ...) WHAT(X) FE_29(WHAT, __VA_ARGS__)

#define GET_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,NAME,...) NAME

#define FOR_EACH(action, ...) \
  GET_MACRO(__VA_ARGS__, FE_30, FE_29, FE_28, FE_27, FE_26, FE_25, FE_24, FE_23, FE_22, FE_21, FE_20, FE_19, FE_18, FE_17, FE_16, FE_15, FE_14, FE_13, FE_12, FE_11, FE_10, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)(action, __VA_ARGS__)

/**
 * Macro: DEFINE_STRUCT_VISITOR_FUNCTION
 *
 * @brief Injects a visitor function into a struct which takes a template lambda, and visits an
 * explicit list of fields.
 * 
 * The lambda should have a syntax like:
 *  `auto lambda = []<typename T>(const char* name, T t) { implement.. }`
 * 
 * @example
 * struct MyStruct {
 *    int32_t id;
 *    std::map<std::string, std::string> props;
 *    std::vector<float> weights;
 * 
 *    DEFINE_STRUCT_VISITOR_FUNCTION(id, props, weights);
 * };
 * 
 * MyStruct s = {42, {{"key", "value"}}, {0.1f, 0.2f}};
 * s._visit_members(lambda);
 * 
 * @note It has two overloads of _visit_members, one for const and one for non-const structs.
 * 
 * @param ... List of member variables to be visited.
 */
#define VISIT_STRUCT_FIELD(field) v(#field, this->field);
#define DEFINE_STRUCT_VISITOR_FUNCTION(...) \
    template <typename V> \
    void _visit_members(V& v) { \
        FOR_EACH(VISIT_STRUCT_FIELD, __VA_ARGS__) \
    } \
    template <typename V> \
    void _visit_members(V& v) const { \
        FOR_EACH(VISIT_STRUCT_FIELD, __VA_ARGS__) \
    }

/**
 * Macro: DEFINE_API_VISITOR_FUNCTION
 *
 * @brief Injects some traits and visitor functions which enables a visitor (a template lambda)
 * to process each api description with their request and response types.
 * 
 * Theere are multiple use cases of this. The simplest one is to get the name and request-response
 * types of each API, which can be done by visiting with a lambda like:
 * 
 * @example
 * auto lambda = []<typename Traits>(Traits t) {
 *   using Req = typename Traits::RequestType;
 *   using Res = typename Traits::ResponseType;
 *   const char* name = Traits::name;
 *   // implement..
 * }
 * 
 * struct GraphApi {
 *     struct AddNodeRequest {
 *         std::string ui_name;
 *         int32_t timestamp;
 *     };
 *     ApiStub<AddNodeRequest, std::string> addNode;
 *     ApiStub<std::string, bool> deleteNode;
 *     DEFINE_API_VISITOR_FUNCTION(addNode, deleteNode);
 * };
 * 
 * api._visit_traits(GraphApi{});
 * 
 * @param ... List of member api descriptors to be visited.
 */

#define API_VISITOR_DEFINE_TRAITS_STRUCT(field) \
    struct field##_traits { \
        using RequestType = typename decltype(field)::RequestType; \
        using ResponseType = typename decltype(field)::ResponseType; \
        static constexpr const char* name = #field; \
    };

#define API_VISITOR_DEFINE_IMPL_PTR(field) \
    typename decltype(field)::ResponseType (T::*field)(const typename decltype(field)::RequestType&) = nullptr;

#define API_VISITOR_DEFINE_VISIT_TRAITS(field) \
    v(field##_traits{});

#define API_VISITOR_DEFINE_VISIT_TRAITS_WITH_IMPL(field) \
    v(field##_traits{}, impl, ptrs.field);

#define DEFINE_API_VISITOR_FUNCTION(...) \
    /* Part 1: Define a trait for each api, containing the name */ \
    FOR_EACH(API_VISITOR_DEFINE_TRAITS_STRUCT, __VA_ARGS__) \
    /* Part 2: Define ImplPtrs struct with function pointers for each API */ \
    template<typename T> struct ImplPtrs { \
        FOR_EACH(API_VISITOR_DEFINE_IMPL_PTR, __VA_ARGS__) \
    }; \
    /* Part 3: Define the _visit_traits function, needs only the traits */ \
    template <typename Visitor> \
    void _visit_traits(Visitor& v) { \
        FOR_EACH(API_VISITOR_DEFINE_VISIT_TRAITS, __VA_ARGS__) \
    } \
    /* Part 4: Define the _visit_traits_with_impl function, needs traits and impl ptrs */ \
    template <typename Impl, typename Visitor> \
    void _visit_traits_with_impl(Visitor& v, Impl& impl, ImplPtrs<Impl>& ptrs) { \
        FOR_EACH(API_VISITOR_DEFINE_VISIT_TRAITS_WITH_IMPL, __VA_ARGS__) \
    }
