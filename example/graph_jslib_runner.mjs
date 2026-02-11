// Execute this as:
// $ bazel run //:graph_jslib_runner

import { loadGraphWasmModule } from './graph_jslib_loader.mjs';

(async () => {
  const module = await loadGraphWasmModule();
  const graph = new module.GraphApi();
  console.log("Loaded GraphApi with apis:", graph.apis);

  let res = graph.addNode({
    ui_name: "Test Node",
    timestamp: Math.floor(Date.now() / 1000),
  });
  console.log("addNode -> ", res);
  const nodeId = res.data;
  res = graph.deleteNode(nodeId);
  console.log("deleteNode -> ", res);
  res = graph.deleteNode(nodeId);
  console.log("deleteNode -> ", res);
})();
