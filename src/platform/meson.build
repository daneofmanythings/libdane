platform_sources = []

system_windows = [
  'windows',
]
system_posix = [
  'linux',
  'darwin',
  'openbsd',
  'freebsd',
]

host_system = host_machine.system()

if host_system in system_posix
  platform_sources += files(
    'src/internal/allocator_wrap.c',
    'src/posix/path.c',
    'src/posix/path_parsing.c',
    'src/posix/thread_local_storage.c',
  )
else
  error('Unsupported platform: ' + host_system)
endif

sources += platform_sources
