// Minimal <inttypes.h> for the freestanding WebAssembly build. littlefs only
// uses the format macros in its debug output, which this build leaves out.
#ifndef LFS_IMAGE_INTTYPES_H
#define LFS_IMAGE_INTTYPES_H
#include <stdint.h>
#define PRIu32 "u"
#define PRId32 "d"
#define PRIx32 "x"
#endif
