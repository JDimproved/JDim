sources = [
  'aboutconfig.cpp',
  'aboutconfigdiag.cpp',
  'configitems.cpp',
  'globalconf.cpp',
]


config_lib = static_library(
  'config', [sources, config_h],
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
