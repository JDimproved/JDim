sources = [
  'gtest_jdlib_cookiemanager.cpp',
  'gtest_jdlib_misctime.cpp',
  'gtest_jdlib_misctrip.cpp',
  'gtest_jdlib_miscutil.cpp',
  'gtest_xml_dom.cpp',
]

deps = [
  gtest_main_dep,
  jdim_deps,
]


test_exe = executable(
  'gtest_jdim', [core_sources, sources],
  dependencies : deps,
  include_directories : jdim_incs,
  link_with : jdim_libs,
)
test('gtest tests', test_exe)
