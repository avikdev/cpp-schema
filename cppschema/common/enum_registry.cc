#include "cppschema/common/enum_registry.h"

EnumRegistry& EnumRegistry::instance() {
    static EnumRegistry instance;
    return instance;
}
