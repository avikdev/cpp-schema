#pragma once

#include <string>
#include <functional>
#include <vector>

#include "cppschema/common/visitor_macros.h"

namespace cppschema {

template <typename Req, typename Res>
struct ApiStub {
    using RequestType = Req;
    using ResponseType = Res;
    const char* name = nullptr;
    void* impl = nullptr;  // Pointer to the actual implementation, can be set by the backend.
};

// Base class for backends to identify themselves
template <typename API>
struct ApiBackend {
    virtual ~ApiBackend() = default;
};

}  // namespace cppschema
