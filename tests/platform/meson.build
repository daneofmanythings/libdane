platform_test_sources = files(
  'unit/parsing_test.c',
  'path_test.c',
  'test_main.c',
  'thread_local_storage_test.c',
)

platform_tests = executable(
  'platform_tests',
  platform_test_sources,
  link_with: libd.get_static_lib(),
)

test(
  'platform tests',
  platform_tests,
  suite: 'platform',
  args: [],
)
