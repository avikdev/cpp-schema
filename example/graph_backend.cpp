#include <iostream>

#include "cppschema/apispec/api_registry.h"
#include "cppschema/backend/api_backend_bridge.h"
#include "graph_api.h"

namespace graph {

class GraphApiImpl : public cppschema::ApiBackend<GraphApi> {
 public:
    std::string addNodeImpl(const GraphApi::AddNodeRequest& request) {
        std::string newId = "node_" + std::to_string(id_counter_++);
        node_storage_[newId] = request.ui_name;
        std::cout << "[Backend] Added node: " << request.ui_name 
                    << " with ID: " << newId 
                    << " at t=" << request.timestamp << std::endl;
        return newId;
    }

    bool deleteNodeImpl(const std::string& id) {
        if (node_storage_.find(id) != node_storage_.end()) {
            node_storage_.erase(id);
            std::cout << "[Backend] Deleted node ID: " << id << std::endl;
            return true;
        }
        std::cout << "[Backend] Delete failed. ID not found: " << id << std::endl;
        return false;
    }

    VoidType clearGraphImpl(const VoidType&) {
        node_storage_.clear();
        std::cout << "[Backend] Cleared all nodes." << std::endl;
        return VoidType{};
    }

 private:
    int32_t id_counter_ = 1000;
    std::map<std::string, std::string> node_storage_;
};

static __attribute__((constructor)) void RegisterGraphApiBackend() {
    auto* impl = new GraphApiImpl();
    GraphApi::ImplPtrs<GraphApiImpl> ptrs = {
        .addNode = &GraphApiImpl::addNodeImpl,
        .deleteNode = &GraphApiImpl::deleteNodeImpl,
        .clearGraph = &GraphApiImpl::clearGraphImpl,
    };
    RegisterBackend<GraphApi, GraphApiImpl>(impl, ptrs);
}

}  // namespace graph
