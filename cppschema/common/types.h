#pragma once

// The sentinel struct representing a C++ `void` type in JavaScript.
// Since `void` cannot be used in references, we use this type in APIs which has no arg, or
// does not return a value.
struct VoidType {};