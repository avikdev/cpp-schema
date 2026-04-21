// Execute this as:
// $ bazel test //:graph_jslib_test

import test from 'node:test';
import assert from 'node:assert/strict';
import { loadGraphWasmModule } from './graph_jslib_loader.mjs';

test.before(async () => {
    console.log("🚀 Loading graph module ...");
    const graphModule = await loadGraphWasmModule();
    if (!graphModule) {
      throw new Error("Failed to load graph WASM module during global setup");
    }
    global.graphModule = graphModule;
    console.log("✅ Installing graph module - OK.");
});

test('WASM Graph Library Test', async (t) => {
  const graph = new graphModule.GraphApi();
  console.assert(!!graph, "GraphApi instance should be created");

  const assertRpcOkAndGetPayload = (rpcResp) => {
    console.log("RPC response -> ", rpcResp);
    assert.ok(rpcResp.ok, `RPC call failed: ${rpcResp.status}`);
    return rpcResp.data;
  }

  await t.test('verify node insert and delete', () => {
    const nodeId = assertRpcOkAndGetPayload(graph.addNode({
      ui_name: "Test Node",
      node_type: "GRAPH_INPUT",
      timestamp: Math.floor(Date.now() / 1000),
    }));
    assert.equal(typeof nodeId, 'string', "Node ID should be a string");
    assert.ok(nodeId.length > 0, "Node ID should be a non-empty string");
    assert.equal(nodeId, 'GRAPH_INPUT_1000', "Unexpected node id");

    const deleteDone1 = assertRpcOkAndGetPayload(graph.deleteNode(nodeId));
    assert.strictEqual(deleteDone1, true, "Node should be deleted successfully");

    const deleteDone2 = assertRpcOkAndGetPayload(graph.deleteNode(nodeId));
    assert.strictEqual(deleteDone2, false, "Deleting the same node again should return false");

    const clearResult = assertRpcOkAndGetPayload(graph.clearGraph({}));
    assert.deepEqual(clearResult, {}, "clearGraph should return null (VoidType)");
  });

  await t.test('verify edges insertion', () => {
    const nodeId1 = assertRpcOkAndGetPayload(graph.addNode({
      ui_name: "Filter Sequence",
      node_type: "FUNCTION",
      timestamp: 1772230000,
    }));
    const nodeId2 = assertRpcOkAndGetPayload(graph.addNode({
      ui_name: "Sum Sequence",
      node_type: "FUNCTION",
      timestamp: 1772230001,
    }));

    assert.equal(nodeId1, 'FUNCTION_1001', "Node ID 1 mismatch");
    assert.equal(nodeId2, 'FUNCTION_1002', "Node ID 2 mismatch");

    const addEdgesReq = {
      entries: [
        { id: 501, source: nodeId1, target: nodeId2 },
        { id: 502, source: nodeId2, target: nodeId1 },
      ]
    };
    const edgeIds = assertRpcOkAndGetPayload(graph.addEdges(addEdgesReq));
    assert.deepEqual(edgeIds, ["edge_501", "edge_502"]);
  });
});