# cpp-schema
A C++20 schema-driven framework for decoupling the core backend logic from Emscripten bindings code
via static reflection macros and type-safe visitor patterns.

This architecture enables clean physical separation between cross-platform logic and WebAssembly-specific glue code. By using a Visitor-based Schema, you define your data once and let the compiler generate Emscripten bindings, without the emscripten binding code directly referencing the backend logic.

The backend layer provides a concrete implementation for the api, and utilizes forced linking (`alwayslink = 1`) of Bazel's C++ library rule (`cc_library`) to ensure backend symbols are available in the Wasm binary.

## Dependency Architecture

```
       [ API-SPEC ] (Header Only)
       /            \
      /              \ (Refers to Schema)
     v                v
[ BACKEND ]         [ WASM ]
(Implementation)  (Bindings Layer)
      |                |
      + ------+------> +
        [ BAZEL DEPS ]
       (Forced Linking)
```

## Code organization

The source directory is `cppschema`.

- **`common`**: Common code. Has preprocessor macros for emulating compile time reflection.
- **`apispec`**: Headers for defining API spec. This should be included by both `backend` and `wasm`.
- **`backend`**: Headers for defining and registering the backend logic.
- **`wasm`**: Headers for generating the binding code based solely on the `apispec`.

The `example` directory has an dummy end-to-end implementation (a complete Bazel module) which can
be used as a reference. It implements some dummy logic to insert and delete nodes in a graph.
See the `BUILD.bazel` file inside for the steps.

- Defines the api schema (rule `graph_api`).
- Registers a dummy backend logic (rule `graph_backend`).
- Generates emscripten binding code (rule `graph_bind`).
- Output wasm binary and glue JS code (rule `graph_wasm`).
- A backend JS test (rule `graph_jslib_test`) and a standalone binary (rule `graph_jslib_runner`)
  which shows how to invoke the WebAssembly code.

## Example use

**Part A**: Define the API

```C++
// File: graph_api.h
#pragma once

#include "cppschema/apispec/api_framework.h"
#include "cppschema/common/visitor_macros.h"

struct GraphApi {
    struct AddNodeRequest {
        std::string ui_name;
        int32_t timestamp;
        DEFINE_STRUCT_VISITOR_FUNCTION(ui_name, timestamp);  // Enables JS conversion.
    };
    // Api-1: Add a node with external name and timestamp, returns the new id.
    ApiStub<AddNodeRequest, std::string> addNode;
    // Api-2: Delete a node by id, returns if actually deleted.
    ApiStub<std::string, bool> deleteNode;

    DEFINE_API_VISITOR_FUNCTION(addNode, deleteNode);
};
```

**Part B**: Register a Backend Impl.

```C++
// File: graph_backend.h
#include "graph_api.h"

class GraphApiImpl : public ApiBackend<GraphApi> {
 public:
    std::string addNodeImpl(const GraphApi::AddNodeRequest& request) { ... }
    bool deleteNodeImpl(const std::string& id) { ... }
};

static __attribute__((constructor)) void RegisterGraphApiImpl() {
    GraphApi::ImplPtrs<GraphApiImpl> ptrs = {
        .addNode = &GraphApiImpl::addNodeImpl,
        .deleteNode = &GraphApiImpl::deleteNodeImpl,
    };
    RegisterBackend<GraphApi, GraphApiImpl>(new GraphApiImpl(), ptrs);
}
```

**Part C**: Emscripten Binding

```C++
#include <emscripten/bind.h>

#include "cppschema/wasm/js_api_bridge.h"
#include "graph_api.h"

EMSCRIPTEN_BINDINGS(my_app) {
    jsbridge::CreateJsApiMethods<GraphApi>("Graph");
}
```

Note that the emscripten binding code has no compile time dependency (does not include)
the backend impl code in `GraphApiImpl`.

**Part D**: Call from Javascript

```javascript
/// Suppose 'mod' is the loaded WASM module.
const graph = new mod.Graph();
const {ok: addOk, data: nodeId} = graph.addNode({
    ui_name: "ModifyGeometry",
    timestamp: 1770530938,
});
console.assert(addOk);
const {ok: deleteApiOk, data: deleted} = graph.deleteNode(nodeId);
console.assert(deleted === true);  // Delete succeeded.
const {ok: deleteApiOk2, data: deleted2} = graph.deleteNode(nodeId);
console.assert(deleted2 === false);  // Already deleted.
```
