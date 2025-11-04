api = files(
  'libd/common.h',
  'libd/errors.h',
  'libd/filesystem.h',
  'libd/memory.h',
  'libd/platform.h',
  'libd/testing.h',
)

platform_api = files(
  'libd/platform/filesystem.h',
  'libd/platform/threads.h',
)

utils_api = files(
  'libd/utils/align_compat.h',
)

libd_api = include_directories('.')

install_headers(api, subdir: 'libd')
install_headers(platform_api, subdir: 'libd/platform')
install_headers(utils_api, subdir: 'libd/utils')
