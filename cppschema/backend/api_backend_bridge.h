#pragma once

#include "cppschema/apispec/api_registry.h"
#include "cppschema/common/visitor_macros.h"

namespace cppschema {

template <typename API, typename Impl>
void RegisterBackend(Impl* instance, const typename API::template ImplPtrs<Impl>& ptrs) {
    auto& registry = ApiRegistry<API>::Get();

    // Ownership Transfer: Store the instance and a type-specific deleter.
    // This wipes any previous backend and its handlers automatically.
    registry.SetBackend(
        static_cast<void*>(instance),
        [](void* ptr) { delete static_cast<Impl*>(ptr); }
    );

    /**
     * Internal Visitor Lambda:
     * This matches the signature expected by API::_visit_traits_with_impl.
     * It captures the 'instance' pointer to create the final dispatch lambdas.
     */
    auto binder = [&](auto stub, auto& impl_ref, auto member_ptr) {
        using Req = typename decltype(stub)::RequestType;
        using Res = typename decltype(stub)::ResponseType;
        const char* name = decltype(stub)::name;

        registry.RegisterHandler(name, [instance, member_ptr](const void* rawReq) -> void* {
            const Req& typedReq = *static_cast<const Req*>(rawReq);
            Res result = (instance->*member_ptr)(typedReq);
            // Return heap-allocated void*
            return static_cast<void*>(new Res(std::move(result)));
        });
    };

    // Use the API's own reflection to drive the registration
    API schema;
    schema._visit_traits_with_impl(binder, *instance, const_cast<typename API::template ImplPtrs<Impl>&>(ptrs));
}

}  // namespace cppschema
