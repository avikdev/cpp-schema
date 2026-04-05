#include "absl/log/log.h"
#include "cppschema/apispec/api_registry.h"
#include "cppschema/backend/api_backend_bridge.h"
#include "graph_api.h"

namespace graph {

std::string NodeTypeToStr(const NodeTypeEnum node_type) {
  switch (node_type) {
    case NodeTypeEnum::UNKNOWN: return "UNKNOWN";
    case NodeTypeEnum::GRAPH_INPUT: return "GRAPH_INPUT";
    case NodeTypeEnum::GRAPH_OUTPUT: return "GRAPH_OUTPUT";
    case NodeTypeEnum::FUNCTION: return "FUNCTION";
  }
}

class GraphApiImpl : public cppschema::ApiBackend<GraphApi> {
 public:
    using AddEdgesRequest = GraphApi::AddEdgesRequest;

    std::string addNodeImpl(const GraphApi::AddNodeRequest& request) {
        std::string new_id = NodeTypeToStr(request.node_type) + "_" + std::to_string(node_counter_++);
        node_storage_[new_id] = request.ui_name;
        LOG(ERROR) << "[Backend] Added node: " << request.ui_name 
                    << " with type: " << NodeTypeToStr(request.node_type)
                    << " with ID: " << new_id
                    << " at t=" << request.timestamp;
        return new_id;
    }

    std::vector<std::string> addEdgesImpl(const AddEdgesRequest& request) {
        std::vector<std::string> result;
        for (const EdgeConnection& conn: request.entries) {
            std::string new_id = "edge_" + std::to_string(conn.id.value);
            edge_storage_[new_id] = conn;
            result.push_back(new_id);
        }
        return result;
    }

    bool deleteNodeImpl(const std::string& id) {
        if (node_storage_.find(id) != node_storage_.end()) {
            node_storage_.erase(id);
            LOG(INFO) << "[Backend] Deleted node ID: " << id;
            return true;
        }
        LOG(INFO) << "[Backend] Delete failed. ID not found: " << id;
        return false;
    }

    VoidType clearGraphImpl(const VoidType&) {
        node_storage_.clear();
        LOG(INFO) << "[Backend] Cleared all nodes";
        return VoidType{};
    }

 private:
    int32_t node_counter_ = 1000;
    std::map<std::string, std::string> node_storage_;
    std::map<std::string, EdgeConnection> edge_storage_;
};

static __attribute__((constructor)) void RegisterGraphApiBackend() {
    auto* impl = new GraphApiImpl();
    GraphApi::ImplPtrs<GraphApiImpl> ptrs = {
        .addNode = &GraphApiImpl::addNodeImpl,
        .addEdges = &GraphApiImpl::addEdgesImpl,
        .deleteNode = &GraphApiImpl::deleteNodeImpl,
        .clearGraph = &GraphApiImpl::clearGraphImpl,
    };
    cppschema::RegisterBackend<GraphApi, GraphApiImpl>(impl, ptrs);
}

}  // namespace graph
