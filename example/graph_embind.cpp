#include <emscripten/bind.h>
#include <emscripten/em_js.h>

#include "cppschema/wasm/js_api_bridge.h"
#include "graph_api.h"

EMSCRIPTEN_BINDINGS(Hello) {
    cppschema::jsbridge::CreateJsApiMethods<graph::GraphApi>("GraphApi");
}
