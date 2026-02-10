#pragma once

#include <emscripten/val.h>

namespace cppschema::jsbridge {

/**
 * JSConverter: A template utility to convert between C++ STL types 
 * and native JavaScript objects/arrays using Embind.
 */
template <typename T, typename Enable = void>
struct JSConverter {
    static emscripten::val toJS(const T& value);
    static T fromJS(emscripten::val v);
};

}  // namespace cppschema::jsbridge

#include "cppschema/wasm/js_converter_inl.h"