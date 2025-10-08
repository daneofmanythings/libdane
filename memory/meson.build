memory_src = files(
  'src/bump_arena.c',
  'src/monotyped_arena.c',

  'src/internal/ring_buffer.c',
)

# Include directory for this library
memory_inc = include_directories('include')

# Build the library (both shared and static)
libmemory = library('libd-memory',
  memory_src,
  include_directories: [inc, memory_inc],
  install: true,
  version: meson.project_version(),
  soversion: '0'
)

# Install headers
install_headers(
  'include/libdane/memory.h',
  subdir: 'libdane'
)

# Create a dependency object for other libraries to use
libmemory_dep = declare_dependency(
  link_with: libmemory,
  include_directories: [inc, memory_inc]
)

# Generate pkg-config file
pkg.generate(libmemory,
  name: 'libd-memory',
  description: 'Memory management library for libdane',
  subdirs: 'libdane'
)

# Tests
# if get_option('enable_tests')
#   test_exe = executable('test_memory',
#     'tests/test_allocator.c',
#     'tests/test_arena.c',
#     include_directories: [inc, memory_inc],
#     link_with: libmemory
#   )
#   
#   test('memory tests', test_exe)
# endif
