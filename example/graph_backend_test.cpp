// Execute this test from the "example" dir as:
// $ bazel test //:graph_backend_test

#include "cppschema/apispec/api_registry.h"
#include "graph_api.h"
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

namespace graph {

using ::cppschema::ApiRegistry;
using ::testing::ElementsAre;

using AddNodeRequest = GraphApi::AddNodeRequest;
using AddEdgesRequest = GraphApi::AddEdgesRequest;

TEST(GraphApiImplTest, Basic) {
    AddNodeRequest add_node_req = {
        .ui_name = "Merge vectors",
        .node_type = NodeTypeEnum::FUNCTION,
        .timestamp = 1772230000,
    };
    std::string node_id0 = ApiRegistry<GraphApi>::Get().template Call<AddNodeRequest, std::string>("addNode", add_node_req);
    EXPECT_EQ(node_id0, "FUNCTION_1000");

    std::string node_id1 = ApiRegistry<GraphApi>::Get().template Call<AddNodeRequest, std::string>("addNode", add_node_req);
    EXPECT_EQ(node_id1, "FUNCTION_1001");

    EdgeConnection conn1 = { .id= EdgeId(101), .source = "FUNCTION_1000", .target = "FUNCTION_1001" };
    EdgeConnection conn2 = { .id= EdgeId(102), .source = "FUNCTION_1001", .target = "FUNCTION_1000" };
    AddEdgesRequest add_edges_req = {
        .entries = { conn1, conn2 },
    };
    std::vector<std::string> edge_ids = ApiRegistry<GraphApi>::Get().template Call<AddEdgesRequest, std::vector<std::string>>("addEdges", add_edges_req);
    EXPECT_THAT(edge_ids, ElementsAre("edge_101", "edge_102"));
}

}  // namespace graph
