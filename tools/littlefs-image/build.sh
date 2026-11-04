#!/bin/sh
# Rebuilds docs/littlefs/lfs-image.wasm from lfs_image.c and littlefs.
#
# Needs clang and wasm-ld with the wasm32 target (on Debian and Ubuntu: the
# clang and lld packages), and git to fetch littlefs.
#
# littlefs is pinned to the release the Battery-Emulator firmware mounts its
# filesystem with: v2.11.3, as bundled by esp_littlefs 1.23.x in the ESP-IDF
# component the Arduino core ships. Bump both together.
set -eu

LITTLEFS_TAG=v2.11.3
LITTLEFS_COMMIT=6cb4e86540eca0d9ba62500a298385c9d863c8be

here=$(cd "$(dirname "$0")" && pwd)
out="$here/../../docs/littlefs/lfs-image.wasm"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

git -c advice.detachedHead=false clone --quiet --depth 1 --branch "$LITTLEFS_TAG" https://github.com/littlefs-project/littlefs.git "$work/littlefs"
actual=$(git -C "$work/littlefs" rev-parse HEAD)
if [ "$actual" != "$LITTLEFS_COMMIT" ]; then
  echo "littlefs $LITTLEFS_TAG is $actual, expected $LITTLEFS_COMMIT" >&2
  exit 1
fi

CFLAGS="--target=wasm32 -O2 -ffreestanding -fno-builtin -nostdlib -I$here/include -I$work/littlefs \
  -DLFS_NO_MALLOC -DLFS_NO_ASSERT -DLFS_NO_DEBUG -DLFS_NO_WARN -DLFS_NO_ERROR"

# shellcheck disable=SC2086
clang $CFLAGS -Wl,--no-entry -Wl,--strip-all -o "$out" \
  "$here/lfs_image.c" "$here/libc.c" "$work/littlefs/lfs.c" "$work/littlefs/lfs_util.c"

chmod 644 "$out"
cp "$work/littlefs/LICENSE.md" "$here/../../docs/littlefs/LICENSE.littlefs.md"
echo "Wrote $out ($(wc -c < "$out") bytes)"
