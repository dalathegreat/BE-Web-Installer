# Board configurations

Board configuration files for the images that take one, by image type. The
installer lists the files of the matching directory under the board name each
file gives, and writes the chosen one together with the firmware.

- `unified_esp32s3/` - for the `UnifiedESP32-S3` image

The files come from `web_data/board-config/` of the Battery-Emulator repository,
which documents the format; `board-config-schema-1.json` is the schema they
follow and is not listed as a board.
