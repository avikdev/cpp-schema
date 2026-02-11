#pragma once

#include <string>
#include <functional>
#include <vector>

#include "cppschema/common/types.h"
#include "cppschema/common/visitor_macros.h"

namespace cppschema {

// This is an empty struct used to convery the types.
template <typename Req, typename Res>
struct ApiStub {
    using RequestType = Req;
    using ResponseType = Res;
};

// Base class for backends to identify themselves
template <typename API>
struct ApiBackend {
    virtual ~ApiBackend() = default;
};

}  // namespace cppschema
