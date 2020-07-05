sources = [
  'buttonconfig.cpp',
  'buttonpref.cpp',
  'control.cpp',
  'controlutil.cpp',
  'keyconfig.cpp',
  'keypref.cpp',
  'mouseconfig.cpp',
  'mousekeyconf.cpp',
  'mousekeypref.cpp',
  'mousepref.cpp',
]


control_lib = static_library(
  'control', [sources, config_h],
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
