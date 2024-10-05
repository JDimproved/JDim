# SPDX-License-Identifier: GPL-2.0-or-later

# Add test code source files to following list.
sources = [
  'gtest_dbimg_imgroot.cpp',
  'gtest_dbtree_nodetreebase.cpp',
  'gtest_dbtree_root.cpp',
  'gtest_dbtree_spchar_decoder.cpp',
  'gtest_jdlib_cookiemanager.cpp',
  'gtest_jdlib_jdiconv.cpp',
  'gtest_jdlib_jdregex.cpp',
  'gtest_jdlib_loader.cpp',
  'gtest_jdlib_misccharcode.cpp',
  'gtest_jdlib_misctime.cpp',
  'gtest_jdlib_misctrip.cpp',
  'gtest_jdlib_miscutil.cpp',
  'gtest_jdlib_span.cpp',
  'gtest_message_messageviewbase.cpp',
  'gtest_xml_dom.cpp',
]

deps = [
  config_h_dep,
  gtest_main_dep,
  jdim_deps,
]


test_exe = executable(
  'gtest_jdim', [buildinfo_h, sources],
  dependencies : deps,
  include_directories : jdim_incs,
  link_with : jdim_libs,
)
test('gtest tests', test_exe)
