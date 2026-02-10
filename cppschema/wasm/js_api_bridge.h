#pragma once

#include <map>

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include "cppschema/apispec/api_framework.h"
#include "cppschema/apispec/api_registry.h"
#include "cppschema/common/visitor_macros.h"
#include "cppschema/wasm/js_converter.h"

namespace cppschema::jsbridge {

template <typename T>
struct ApiResponseOrError {
    // std::optional<T> data;
    T data;
    bool ok = false;
    // Can be "OK", or error message.
    std::string status;

    DEFINE_STRUCT_VISITOR_FUNCTION(data, ok, status);
};

struct ApiInfo {
    // The api name.
    std::string name;
    // TODO: Currently these are populated with typeid names, which are C++ mangled names not
    // quite readable. In future derive readable type description using the custom type traits.
    std::string req;
    std::string resp;

    // This is surfaced to JS via the `apis` property.
    DEFINE_STRUCT_VISITOR_FUNCTION(name, req, resp);
};

template <typename API>
struct EmClazz {
    std::map<std::string, ApiInfo> api_infos;  // Collected api infos.

    static EmClazz& Get() {
        static EmClazz instance;
        return instance;
    }

    void addApiInfo(ApiInfo&& api) {
        api_infos[api.name] = std::move(api);
    }

    emscripten::val getApiInfosAsJsVal() const {
        const auto& api_infos = EmClazz::Get().api_infos;
        emscripten::val arr = emscripten::val::array();
        for (const auto& [k, v] : api_infos) {
            arr.call<void>("push", JSConverter<ApiInfo>::toJS(v));
        }
        return arr;
    }
};

template <typename API>
struct JsDispatchVisitor {
    using ApiClazz = EmClazz<API>;
    emscripten::class_<ApiClazz> clazz;

    JsDispatchVisitor(const std::string& alias) : clazz(alias.c_str()) {
        clazz.template constructor<>();
    }

    ~JsDispatchVisitor() {
        clazz.property("apis", &ApiClazz::getApiInfosAsJsVal);
    }

    template <typename Traits>
    void operator()(Traits traits) {
        std::string methodName = Traits::name;
        ApiClazz::Get().addApiInfo({
            .name = methodName,
            .req = typeid(typename Traits::RequestType).name(),
            .resp = typeid(typename Traits::ResponseType).name()
        });

        clazz.function(methodName.c_str(), emscripten::optional_override(
                [](ApiClazz& self, emscripten::val jsArgs) -> emscripten::val {
            using Req = typename Traits::RequestType;
            using Res = typename Traits::ResponseType;
            const char* name = Traits::name;

            ApiResponseOrError<Res> response;
            Req cppReq = JSConverter<Req>::fromJS(jsArgs);
            // Dispatch to  Registry. Looks up the type-erased handler to execute backend logic.
            response.data = ApiRegistry<API>::Get().template Call<Req, Res>(name, cppReq);
            response.ok = true;
            response.status = "ok";
            // TODO: Handle error.
            if (false) {
                // Handle logic errors or missing backend registration
                response.ok = false;
                response.status = "Error message here";
            }
            // 4. Convert C++ Response Struct -> JS Object
            return JSConverter<ApiResponseOrError<Res>>::toJS(response);
        }));
    }
};

/**
 * The entry point for Emscripten bindings.
 */
template <typename API>
void CreateJsApiMethods(const std::string& alias) {
    API skeleton;
    // Drive the visitor to attach methods to the JS object
    JsDispatchVisitor<API> visitor(alias);
    skeleton._visit_traits(visitor);
}

}  // namespace cppschema::jsbridge
