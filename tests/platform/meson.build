platform_test_sources = files(
  'test_main.c',
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
  args: [
    test_args,
  ],
)
