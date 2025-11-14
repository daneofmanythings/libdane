#ifndef FILESYSTEM_PATH_PLATFORM_WRAP_H
#define FILESYSTEM_PATH_PLATFORM_WRAP_H

#include "../../../include/libd/filesystem.h"

#include <stdbool.h>

usize
platform_filepath_prefix_len(const char* out_path);

#endif  // FILESYSTEM_PATH_PLATFORM_WRAP_H
