filesystem_test_internal_includes = include_directories(
  '../../src/filesystem/',
  '../../src/filesystem/internal/',
)

filesystem_test_sources = files(
  'test_main.c',
)

filesystem_tests = executable(
  'filesystem_tests',
  filesystem_test_sources,
  link_with: libd.get_static_lib(),
  include_directories: [
    filesystem_test_internal_includes,
  ],
)

test(
  'filesystem tests',
  filesystem_tests,
  suite: 'filesystem',
  args: [],
)
