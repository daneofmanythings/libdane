platform_includes = include_directories('include')

# tracking headers for rebuilding
platform_sources = files(
  'include/libdane/platform.h',
  'include/libdane/platform/thread.h',
)

if host_system in system_posix
  platform_sources += files(
    'src/posix/thread/thread.c',
  )
else
  error('Unsupported platform: ' + host_system)
endif

libd_platform= library('libd-platform',
  platform_sources,
  include_directories: [
    platform_includes,
  ],
  install: true,
  version: meson.project_version(),
  soversion: '0'
)

# main header
install_headers(
  'include/libdane/platform.h',
  subdir: 'libdane'
)

# sub-headers
install_headers(
  'include/libdane/platform/thread.h',
  subdir: 'libdane/platform'
)

# Create a dependency object for other libraries to use
libd_platform_dep = declare_dependency(
  link_with: libd_platform,
  include_directories: [
    platform_includes,
  ]
)

if build_tests
  test_sources = files(
    'tests/thread_test.c',
  )

  platform_test = executable('test_platform',
    test_sources,
    dependencies: [
      libd_platform_dep,
      criterion_dep,
    ],
  )
  
  test('platform tests', platform_test)
endif
