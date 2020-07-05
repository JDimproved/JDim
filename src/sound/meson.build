sources = [
  'playsound.cpp',
  'soundmanager.cpp',
]

deps = [
  alsa_dep,
  gtkmm_dep,
]


sound_lib = static_library(
  'sound', [sources, config_h],
  dependencies : deps,
  include_directories : include_directories('..'),
)
