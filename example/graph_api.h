#pragma once

#include "cppschema/apispec/api_framework.h"
#include "cppschema/common/visitor_macros.h"

namespace graph {

struct GraphApi {
    struct AddNodeRequest {
        std::string ui_name;
        int32_t timestamp;

        DEFINE_STRUCT_VISITOR_FUNCTION(ui_name, timestamp);
    };
    // Add a node with external name and timestamp, returns the new id.
    cppschema::ApiStub<AddNodeRequest, std::string> addNode;
    // Delete a node by id, returns if successfully deleted.
    cppschema::ApiStub<std::string, bool> deleteNode;

    DEFINE_API_VISITOR_FUNCTION(addNode, deleteNode);
};

}  // namespace graph
