sources = [
  'playsound.cpp',
  'soundmanager.cpp',
]

deps = [
  alsa_dep,
  config_h_dep,
  gtkmm_dep,
]


sound_lib = static_library(
  'sound', sources,
  dependencies : deps,
  include_directories : include_directories('..'),
)
