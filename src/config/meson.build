sources = [
  'aboutconfig.cpp',
  'aboutconfigdiag.cpp',
  'configitems.cpp',
  'globalconf.cpp',
]


config_lib = static_library(
  'config', sources,
  dependencies : [config_h_dep, gtkmm_dep],
  include_directories : include_directories('..'),
)
