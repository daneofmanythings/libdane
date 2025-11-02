errors_includes = include_directories('include')

# tracking headers for rebuilding
errors_sources = files(
  'include/libdane/errors.h',
  'src/internal/internal.h',
)

if host_system in system_posix
  errors_sources += files(
    'src/errors.c',
    'src/internal/platform_wrap.c',
  )
else
  error('Unsupported platform: ' + host_system)
endif

libd_errors= library('libd-errors',
  errors_sources,
  dependencies: [
    libd_platform_dep,
  ],
  include_directories: [
    errors_includes,
  ],
  install: true,
  version: meson.project_version(),
  soversion: '0'
)

# main header
install_headers(
  'include/libdane/errors.h',
  subdir: 'libdane'
)

# Create a dependency object for other libraries to use
libd_errors_dep = declare_dependency(
  link_with: libd_errors,
  dependencies: [
    core_includes,
    libd_platform_dep,
  ],
  include_directories: [
    errors_includes,
  ]
)

if build_tests
  test_sources = files(
    'tests/errors_test.c',
  )

  errors_test = executable('test_errors',
    test_sources,
    dependencies: [
      libd_errors_dep,
      criterion_dep,
    ],
  )

  args = ['--verbose']
  
  test('errors tests', errors_test, args: args)
endif
