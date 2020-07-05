sources = [
  'confloader.cpp',
  'cookiemanager.cpp',
  'heap.cpp',
  'imgloader.cpp',
  'jdiconv.cpp',
  'jdmigemo.cpp',
  'jdregex.cpp',
  'jdthread.cpp',
  'loader.cpp',
  'loaderdata.cpp',
  'misccharcode.cpp',
  'miscgtk.cpp',
  'miscmsg.cpp',
  'misctime.cpp',
  'misctrip.cpp',
  'miscutil.cpp',
  'miscx.cpp',
  'ssl.cpp',
  'tfidf.cpp',
  'timeout.cpp',
]

deps = [
  gtkmm_dep,
  socket_dep,
  tls_dep,
  x11_dep,
]


jdlib_lib = static_library(
  'jdlib', [sources, config_h],
  dependencies : deps,
  include_directories : include_directories('..'),
)
