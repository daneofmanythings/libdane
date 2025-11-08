memory_internal_includes = include_directories(
  '../../src/memory/internal/',
)

memory_test_sources = files(
  'test_main.c',
)

memory_tests = executable(
  'memory_tests',
  memory_test_sources,
  link_with: libd.get_static_lib(),
  include_directories: [
    memory_internal_includes,
  ],
)

test(
  'memory tests',
  memory_tests,
  suite: 'memory',
  args: [
    test_args,
  ],
)
