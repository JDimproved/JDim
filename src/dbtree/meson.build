# SPDX-License-Identifier: GPL-2.0-or-later

sources = [
  'article2ch.cpp',
  'article2chcompati.cpp',
  'articlebase.cpp',
  'articlehash.cpp',
  'articlejbbs.cpp',
  'articlelocal.cpp',
  'articlemachi.cpp',
  'bbsmenu.cpp',
  'board2ch.cpp',
  'board2chcompati.cpp',
  'boardbase.cpp',
  'boardfactory.cpp',
  'boardjbbs.cpp',
  'boardlocal.cpp',
  'boardmachi.cpp',
  'frontloader.cpp',
  'interface.cpp',
  'nodetree2ch.cpp',
  'nodetree2chcompati.cpp',
  'nodetreebase.cpp',
  'nodetreedummy.cpp',
  'nodetreejbbs.cpp',
  'nodetreelocal.cpp',
  'nodetreemachi.cpp',
  'root.cpp',
  'ruleloader.cpp',
  'settingloader.cpp',
  'spchar_decoder.cpp',
]


dbtree_lib = static_library(
  'dbtree', sources,
  dependencies : gtkmm_dep,
  include_directories : include_directories('..'),
)
