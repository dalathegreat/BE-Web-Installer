// Builds a LittleFS partition image in the browser, holding one file. The work
// is done by littlefs itself, compiled to WebAssembly from
// tools/littlefs-image/ - see the build script there for the exact release.

let modulePromise = null;

function loadModule() {
  if (!modulePromise) {
    const url = new URL("lfs-image.wasm", import.meta.url);
    modulePromise = fetch(url)
      .then((r) => {
        if (!r.ok) {
          throw new Error(`Loading ${url} failed: ${r.status}`);
        }
        return r.arrayBuffer();
      })
      .then((bytes) => WebAssembly.instantiate(bytes, {}))
      .then((result) => result.instance.exports)
      .catch((e) => {
        modulePromise = null; // let a later call try again
        throw e;
      });
  }
  return modulePromise;
}

// Returns a Uint8Array of exactly partitionSize bytes: a formatted LittleFS
// filesystem with `data` (a Uint8Array) stored as `name` at its root.
export async function buildLittlefsImage(name, data, partitionSize) {
  const lfs = await loadModule();
  const blockSize = lfs.lfsimg_block_size();
  if (partitionSize % blockSize !== 0) {
    throw new Error(`Partition size ${partitionSize} is not a multiple of ${blockSize}`);
  }
  const nameBytes = new TextEncoder().encode(name);
  if (nameBytes.length > lfs.lfsimg_name_max()) {
    throw new Error(`File name "${name}" is too long`);
  }
  if (data.length > lfs.lfsimg_data_max()) {
    throw new Error(`File is too large (${data.length} bytes)`);
  }

  const memory = new Uint8Array(lfs.memory.buffer);
  const namePtr = lfs.lfsimg_name();
  memory.set(nameBytes, namePtr);
  memory[namePtr + nameBytes.length] = 0;
  memory.set(data, lfs.lfsimg_data());

  const blockCount = partitionSize / blockSize;
  const mtime = Math.floor(Date.now() / 1000) >>> 0;
  const err = lfs.lfsimg_build(blockCount, data.length, mtime);
  if (err !== 0) {
    throw new Error(`Building the filesystem image failed (littlefs error ${err})`);
  }
  // Copy out: the next build reuses the same memory.
  const imagePtr = lfs.lfsimg_image();
  return new Uint8Array(lfs.memory.buffer, imagePtr, partitionSize).slice();
}
