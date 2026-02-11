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

  await t.test('verify the dummy backend logic worked', () => {
    const nodeId = assertRpcOkAndGetPayload(graph.addNode({
      ui_name: "Test Node",
      timestamp: Math.floor(Date.now() / 1000),
    }));
    assert.equal(typeof nodeId, 'string', "Node ID should be a string");
    assert.ok(nodeId.length > 0, "Node ID should be a non-empty string");

    const deleteDone1 = assertRpcOkAndGetPayload(graph.deleteNode(nodeId));
    assert.strictEqual(deleteDone1, true, "Node should be deleted successfully");

    const deleteDone2 = assertRpcOkAndGetPayload(graph.deleteNode(nodeId));
    assert.strictEqual(deleteDone2, false, "Deleting the same node again should return false");

    const clearResult = assertRpcOkAndGetPayload(graph.clearGraph({}));
    assert.deepEqual(clearResult, {}, "clearGraph should return null (VoidType)");
  });
});