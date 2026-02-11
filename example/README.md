# Usage Example

This directory has a dummy end-to-end implementation (a complete Bazel module) which can
be used as a reference. It implements some dummy logic to insert and delete nodes in a graph.

This assume a dummy Graph api with these methods:
- **AddNode:** Add a new node to the graph, returns the id.
- **DeleteNode:** Deletes a node by id, returns true if actually deleted.
- **ClearGrah** Clear all nodes.

See the `BUILD.bazel` file inside for rules of the different steps:

- Defines the api schema (rule `graph_api`).
- Registers a dummy backend logic (rule `graph_backend`).
- Generates emscripten binding code (rule `graph_bind`).
- Output wasm binary and glue JS code (rule `graph_wasm`).
- A backend JS test (rule `graph_jslib_test`) and a standalone binary (rule `graph_jslib_runner`)
  which shows how to invoke the WebAssembly code.

Run the javascript test which builds everything end-to-end all the way upto the wasm binary and
tests the backed logic inside it.

```
$ bazel test //:graph_jslib_test
```