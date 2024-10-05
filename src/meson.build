# SPDX-License-Identifier: GPL-2.0-or-later

config_h = configure_file(output : 'config.h',
                          configuration : conf)
config_h_dep = declare_dependency(compile_args : '-DHAVE_CONFIG_H=1',
                                  include_directories : include_directories('.'),
                                  sources: config_h)

# 作業ディレクトリの状態を反映するためにビルド毎に更新する
buildinfo_h = custom_target('buildinfo.h',
                            build_always_stale : true,
                            command : ['sh', '@INPUT@', '@OUTPUT@', '@SOURCE_ROOT@'],
                            input : 'buildinfo.h.sh',
                            output : 'buildinfo.h')


subdir('article')
subdir('bbslist')
subdir('board')
subdir('config')
subdir('control')
subdir('dbimg')
subdir('dbtree')
subdir('history')
subdir('icons')
subdir('image')
subdir('jdlib')
subdir('message')
subdir('skeleton')
subdir('sound')
subdir('xml')


core_sources = [
  'aamanager.cpp',
  'articleitemmenupref.cpp',
  'articleitempref.cpp',
  'boarditemmenupref.cpp',
  'boarditempref.cpp',
  'browsers.cpp',
  'cache.cpp',
  'command.cpp',
  'compmanager.cpp',
  'core.cpp',
  'cssmanager.cpp',
  'dispatchmanager.cpp',
  'dndmanager.cpp',
  'environment.cpp',
  'fontcolorpref.cpp',
  'iomonitor.cpp',
  'linkfiltermanager.cpp',
  'linkfilterpref.cpp',
  'livepref.cpp',
  'login2ch.cpp',
  'loginbe.cpp',
  'loginacorn.cpp',
  'mainitempref.cpp',
  'maintoolbar.cpp',
  'menuslots.cpp',
  'msgitempref.cpp',
  'openurldiag.cpp',
  'prefdiagfactory.cpp',
  'replacestrmanager.cpp',
  'replacestrpref.cpp',
  'searchitempref.cpp',
  'searchloader.cpp',
  'searchmanager.cpp',
  'session.cpp',
  'setupwizard.cpp',
  'sharedbuffer.cpp',
  'sidebaritempref.cpp',
  'updatemanager.cpp',
  'urlreplacemanager.cpp',
  'usrcmdmanager.cpp',
  'usrcmdpref.cpp',
  'viewfactory.cpp',
  'winmain.cpp',
]

# テストプログラムのビルドで利用するためライブラリにまとめる
core_lib = static_library(
  'core', core_sources,
  dependencies : [config_h_dep, gtkmm_dep, tls_dep],
  include_directories : include_directories('.'),
)


sources = [
  'main.cpp',
]

jdim_deps = [
  alsa_dep,
  crypt_dep,
  gtkmm_dep,
  ice_dep,
  migemo_dep,
  sm_dep,
  socket_dep,
  threads_dep,
  tls_dep,
  x11_dep,
  zlib_dep,
]

jdim_incs = include_directories('.')

jdim_libs = [
  article_lib,
  bbslist_lib,
  board_lib,
  config_lib,
  control_lib,
  core_lib,
  dbimg_lib,
  dbtree_lib,
  history_lib,
  icon_lib,
  image_lib,
  jdlib_lib,
  message_lib,
  skeleton_lib,
  sound_lib,
  xml_lib,
]


jdim_exe = executable(
  'jdim', [sources, buildinfo_h],
  dependencies : [config_h_dep, jdim_deps],
  include_directories : jdim_incs,
  link_with : jdim_libs,
  install : true,
)
