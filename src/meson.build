sources = []
internal_includes = []

libs = [
  'memory',
  'filesystem',
  # 'platform',
  # 'errors',
]

foreach lib : libs
  subdir(lib)
endforeach

libd = library(
  'libd',
  sources,
  dependencies: [],
  include_directories: [
    libd_includedirs,
    internal_includes,
  ],
  install: true,
  version: meson.project_version(),
  soversion: '0',
)

libd_dep = declare_dependency(
  link_with: libd,
  dependencies: [],
  include_directories: [
    libd_api,
  ],
)
