#pragma once

#include <string>
#include <vector>

#include "cppschema/apispec/api_framework.h"
#include "cppschema/common/types.h"
#include "cppschema/common/visitor_macros.h"

namespace graph {

struct EdgeConnection {
    std::string source;
    std::string target;

    DEFINE_STRUCT_VISITOR_FUNCTION(source, target);
};

struct GraphApi {
    struct AddNodeRequest {
        std::string ui_name;
        int32_t timestamp;

        DEFINE_STRUCT_VISITOR_FUNCTION(ui_name, timestamp);
    };

    struct AddEdgesRequest {
        std::vector<EdgeConnection> entries;

        DEFINE_STRUCT_VISITOR_FUNCTION(entries);
    };

    // Add a node with external name and timestamp, returns the new id.
    cppschema::ApiStub<AddNodeRequest, std::string> addNode;
    // Add one or more edges, returns the new edge ids.
    cppschema::ApiStub<AddEdgesRequest, std::vector<std::string>> addEdges;
    // Delete a node by id, returns if successfully deleted.
    cppschema::ApiStub<std::string, bool> deleteNode;
    // Clears all data.
    cppschema::ApiStub<VoidType, VoidType> clearGraph;

    DEFINE_API_VISITOR_FUNCTION(addNode, addEdges, deleteNode, clearGraph);
};

}  // namespace graph
