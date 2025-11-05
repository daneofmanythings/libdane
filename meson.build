project(
  'libd',
  'c',
  version: '0.1.0',
  default_options: [
    'warning_level=3',
    'c_std=c99',
    'default_library=both',
  ],
)

libd_includedirs = include_directories(
  'include',
)

subdir('include')
subdir('src')

if get_option('tests')
  subdir('tests')
endif
