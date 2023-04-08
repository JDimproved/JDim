sources = [
  'confloader.cpp',
  'cookiemanager.cpp',
  'heap.cpp',
  'imgloader.cpp',
  'jdiconv.cpp',
  'jdmigemo.cpp',
  'jdregex.cpp',
  'jdsocket.cpp',
  'loader.cpp',
  'loaderdata.cpp',
  'misccharcode.cpp',
  'miscgtk.cpp',
  'miscmsg.cpp',
  'misctime.cpp',
  'misctrip.cpp',
  'miscutil.cpp',
  'miscx.cpp',
  'tfidf.cpp',
  'timeout.cpp',
]

deps = [
  config_h_dep,
  gtkmm_dep,
  socket_dep,
  tls_dep,
  x11_dep,
]


jdlib_lib = static_library(
  'jdlib', sources,
  dependencies : deps,
  include_directories : include_directories('..'),
)
