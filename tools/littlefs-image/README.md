# littlefs-image

The installer writes a board configuration next to the unified firmware as a
LittleFS image of the board's `spiffs` partition, built in the browser from the
JSON the user picked. `lfs_image.c` does that with littlefs itself, compiled to
WebAssembly, so the image comes from the same littlefs release the firmware
mounts it with, not from a second implementation of the format.

- `lfs_image.c` - formats an image and stores one file in it
- `libc.c`, `include/` - the few C library pieces littlefs needs, since the
  WebAssembly build has no libc
- `build.sh` - fetches the pinned littlefs release and writes
  `docs/littlefs/lfs-image.wasm`, plus littlefs' licence next to it

`docs/littlefs/lfs-image.js` is the JavaScript side the page imports.

Rebuild after changing anything here, or when the firmware moves to another
littlefs release (update `LITTLEFS_TAG` and `LITTLEFS_COMMIT` in `build.sh`):

    tools/littlefs-image/build.sh

It needs clang and wasm-ld with the wasm32 target (Debian and Ubuntu: the
`clang` and `lld` packages) and git. The output is reproducible: the same
inputs give the same `lfs-image.wasm`.
