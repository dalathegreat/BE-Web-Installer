// Builds a LittleFS partition image in memory, for the web installer to flash
// next to the firmware. Compiled to WebAssembly by build.sh; docs/littlefs/
// lfs-image.js is the JavaScript side.
//
// The image is made by the same littlefs release the firmware mounts it with
// (see build.sh), so there is no second implementation of the on-disk format
// that could drift from the real one.
//
// Usage from JavaScript: copy the file name and contents into the buffers that
// lfsimg_name() and lfsimg_data() point to, call lfsimg_build(), then read
// block_size * block_count bytes from lfsimg_image().

#include "lfs.h"

#define BLOCK_SIZE 4096
#define MAX_BLOCKS 1024  // 4 MiB, more than any partition the installer writes
#define CACHE_SIZE 256
#define LOOKAHEAD_SIZE 32
#define NAME_MAX 64  // what PlatformIO's buildfs records, and below the firmware's 255
#define MAX_DATA (64 * 1024)

static uint8_t image[BLOCK_SIZE * MAX_BLOCKS];
static uint8_t read_buffer[CACHE_SIZE];
static uint8_t prog_buffer[CACHE_SIZE];
static uint8_t lookahead_buffer[LOOKAHEAD_SIZE];
static uint8_t file_buffer[CACHE_SIZE];
static char name_buffer[NAME_MAX + 1];
static uint8_t data_buffer[MAX_DATA];

static int bd_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size) {
  memcpy(buffer, image + block * c->block_size + off, size);
  return 0;
}

static int bd_prog(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size) {
  memcpy(image + block * c->block_size + off, buffer, size);
  return 0;
}

static int bd_erase(const struct lfs_config* c, lfs_block_t block) {
  memset(image + block * c->block_size, 0xff, c->block_size);
  return 0;
}

static int bd_sync(const struct lfs_config* c) {
  (void)c;
  return 0;
}

__attribute__((export_name("lfsimg_image"))) uint8_t* lfsimg_image(void) {
  return image;
}

__attribute__((export_name("lfsimg_name"))) char* lfsimg_name(void) {
  return name_buffer;
}

__attribute__((export_name("lfsimg_name_max"))) uint32_t lfsimg_name_max(void) {
  return NAME_MAX;
}

__attribute__((export_name("lfsimg_data"))) uint8_t* lfsimg_data(void) {
  return data_buffer;
}

__attribute__((export_name("lfsimg_data_max"))) uint32_t lfsimg_data_max(void) {
  return MAX_DATA;
}

__attribute__((export_name("lfsimg_block_size"))) uint32_t lfsimg_block_size(void) {
  return BLOCK_SIZE;
}

// Formats an image of block_count blocks holding one file: name_buffer (NUL
// terminated, at the root), with data_length bytes of data_buffer and an
// ESP-IDF style modification time attribute. Returns 0, or a negative littlefs
// error code.
__attribute__((export_name("lfsimg_build"))) int lfsimg_build(uint32_t block_count, uint32_t data_length,
                                                             uint32_t mtime) {
  if (block_count < 2 || block_count > MAX_BLOCKS || data_length > MAX_DATA) {
    return LFS_ERR_INVAL;
  }
  name_buffer[NAME_MAX] = '\0';

  // The whole image starts erased, as a freshly erased flash partition would.
  memset(image, 0xff, (size_t)block_count * BLOCK_SIZE);

  struct lfs_config cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.read = bd_read;
  cfg.prog = bd_prog;
  cfg.erase = bd_erase;
  cfg.sync = bd_sync;
  cfg.read_size = 16;
  cfg.prog_size = 16;
  cfg.block_size = BLOCK_SIZE;
  cfg.block_count = block_count;
  cfg.block_cycles = 512;
  cfg.cache_size = CACHE_SIZE;
  cfg.lookahead_size = LOOKAHEAD_SIZE;
  cfg.read_buffer = read_buffer;
  cfg.prog_buffer = prog_buffer;
  cfg.lookahead_buffer = lookahead_buffer;
  cfg.name_max = NAME_MAX;

  lfs_t lfs;
  int err = lfs_format(&lfs, &cfg);
  if (err) {
    return err;
  }
  err = lfs_mount(&lfs, &cfg);
  if (err) {
    return err;
  }

  // 't' is the attribute esp_littlefs keeps a file's modification time in,
  // which is also what PlatformIO's buildfs writes.
  uint8_t mtime_bytes[4] = {(uint8_t)mtime, (uint8_t)(mtime >> 8), (uint8_t)(mtime >> 16), (uint8_t)(mtime >> 24)};
  struct lfs_attr attrs[] = {{'t', mtime_bytes, sizeof(mtime_bytes)}};
  struct lfs_file_config file_cfg;
  memset(&file_cfg, 0, sizeof(file_cfg));
  file_cfg.buffer = file_buffer;
  file_cfg.attrs = attrs;
  file_cfg.attr_count = 1;

  lfs_file_t file;
  err = lfs_file_opencfg(&lfs, &file, name_buffer, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC, &file_cfg);
  if (err) {
    lfs_unmount(&lfs);
    return err;
  }
  lfs_ssize_t written = lfs_file_write(&lfs, &file, data_buffer, data_length);
  int close_err = lfs_file_close(&lfs, &file);
  int unmount_err = lfs_unmount(&lfs);
  if (written < 0) {
    return (int)written;
  }
  if ((uint32_t)written != data_length) {
    return LFS_ERR_NOSPC;
  }
  return close_err ? close_err : unmount_err;
}
