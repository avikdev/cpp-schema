#pragma once

// This sentinel struct represents a C++ `void` type with an empty object JavaScript.
// Since `void` cannot be used in references, we use this type in APIs which has no arg, or
// does not return a value.
struct VoidType {};
