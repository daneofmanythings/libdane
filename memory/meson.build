memory_includes = include_directories('include')

# tracking headers for rebuilding
memory_sources = files(
  'include/libdane/memory.h',
)

# TODO: verify platform specifics
if host_system in system_posix
  memory_sources += files(
    'src/linear_allocator.c',
    'src/pool_allocator.c',
  )
else
  error('Unsupported platform: ' + host_system)
endif

libd_memory= library('libd-memory',
  memory_sources,
  include_directories: [
    memory_includes,
  ],
  install: true,
  version: meson.project_version(),
  soversion: '0'
)

# main header
install_headers(
  'include/libdane/memory.h',
  subdir: 'libdane'
)

# Create a dependency object for other libraries to use
libd_memory_dep = declare_dependency(
  link_with: libd_memory,
  include_directories: [
    memory_includes,
  ]
)

if build_tests
  test_sources = files(
    'tests/linear_allocator_test.c',
    'tests/pool_allocator_test.c',
  )

  memory_test = executable('test_memory',
    test_sources,
    dependencies: [
      libd_memory_dep,
      criterion_dep,
    ],
  )

  args = ['--verbose']
  
  test('memory tests', memory_test, args: args)
endif
