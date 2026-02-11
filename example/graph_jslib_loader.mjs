import fs from "fs";
import path from "path";
import process from "process";

// Bazel runfiles and workspace exposed via env
const runfiles = process.env.JS_BINARY__RUNFILES || "";
const workspace = process.env.JS_BINARY__WORKSPACE || "";

async function loadGraphWasmModule() {
  if (runfiles.length <= 0 || workspace.length <= 0) {
    console.error("Empty env params: ", {runfiles, workspace});
    process.exit(1);
  }
  const wasmDir = path.join(runfiles, workspace, "graph_wasm");
  const wasmBinaryPath = path.join(wasmDir, "graph_bind.wasm");
  const glueJsPath = path.join(wasmDir, "graph_bind.js");

  const binaryStream = fs.ReadStream(wasmBinaryPath);
  const { default: WasmModule } = await import(glueJsPath);
  const module = await WasmModule({ binaryStream });
  if (!module) {
    console.error("Failed to load WASM module");
    process.exit(1);
  }
  return module;
}

export { loadGraphWasmModule };