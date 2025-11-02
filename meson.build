project('libdane', 'c',
  version: '0.1.0',
  default_options: [
    'warning_level=3',
    'c_std=c99',
    'default_library=both', 
  ]
)

build_tests = get_option('tests')

if build_tests
  criterion_dep = dependency('criterion', required: true)
endif

host_system = host_machine.system()

core_includes = include_directories('include')

system_windows = [
  'windows',
]

system_posix = [
  'linux',
  'darwin',
  'openbsd',
  'freebsd',
]

libs = [
  'memory',
  'platform',
  'errors',
]

foreach lib: libs
  subdir(lib)
endforeach

