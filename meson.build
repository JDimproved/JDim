# SPDX-License-Identifier: GPL-2.0-or-later
# JDim用 meson.build

# Autotoolsからの移行はGNOMEのガイドラインを参考にする
# https://wiki.gnome.org/Initiatives/GnomeGoals/MesonPorting

# mesonを使ってJDimをビルドする方法
#
# Fedora
#   ディストロのパッケージをインストールする
#   ```
#   dnf install git meson
#   dnf install gnutls-devel gtkmm30-devel libSM-devel
#   ```
#
# Debian bullseye, Ubuntu 22.04
#   ディストロのパッケージをインストールする
#   ```
#   sudo apt install build-essential git meson
#   sudo apt install libgnutls28-dev libgtkmm-3.0-dev libltdl-dev
#   ```

# ビルドの手順
#   ```
#   git clone -b master --depth 1 https://github.com/JDimproved/JDim.git jdim
#   cd jdim
#   meson setup builddir
#   cd builddir
#   ninja
#   ```
# Tips
#   - JDimのビルドオプションは `meson configure` を実行してProject optionsの段落を確認する
#     または meson_options.txt を見る
#   - ビルドオプションは `meson setup builddir -Dpangolayout=enabled` のように指定する
#   - 生成された実行ファイルの場所は builddir/src/jdim

project('jdim', 'cpp',
        version : '0.14.0-beta',
        license : 'GPL2',
        meson_version : '>= 0.61.0',
        default_options : ['warning_level=3', 'cpp_std=c++17'])

# 追加コンパイルオプション
# -Wextraで有効になる-Wunused-parameterは修正方法の検討が必要なので暫定的に無効
add_project_arguments('-Wno-unused-parameter', language : 'cpp')

cpp_compiler = meson.get_compiler('cpp')


conf = configuration_data()
# オプションの表示にはconfigureスタイル('--with-foo')が必要なので注意
configure_args = []


#
# ビルドされたバイナリが実行されるマシンの情報
#
if host_machine.endian() == 'big'
  conf.set('WORDS_BIGENDIAN', 1)
endif


#
# 必須パッケージのチェック
#
gtkmm_dep = dependency('gtkmm-3.0', version : '>= 3.24.2')
gtkmm_dep = declare_dependency(compile_args : '-DGTK_DOMAIN="gtk30"',
                               dependencies : gtkmm_dep)
threads_dep = dependency('threads')
x11_dep = dependency('x11')
zlib_dep = dependency('zlib', version : '>= 1.2.0')


# crypt(3)
crypt_dep = cpp_compiler.find_library('crypt', required : false)
if not crypt_dep.found()
  crypt_dep = dependency('libcrypt', required : false)
endif
if not crypt_dep.found()
  crypt_dep = dependency('libxcrypt', required : false)
endif
# OpenBSD's crypt(3) is provided by libc.
if not crypt_dep.found() and not cpp_compiler.has_function('crypt')
  error('crypt(3) is not found... Please report to <https://github.com/JDimproved/JDim/issues>')
endif

if cpp_compiler.has_function('crypt_r', dependencies : crypt_dep)
  conf.set('HAVE_CRYPT_R', 1)
endif

# socket
if cpp_compiler.has_header('sys/socket.h')
  socket_dep = dependency('', required : false)
else
  socket_dep = cpp_compiler.find_library('socket')
endif

if not cpp_compiler.has_function('timegm')
  conf.set('NO_TIMEGM', 1)
endif


#
# オプションのパッケージ
#

# セッション管理
sessionlib_opt = get_option('sessionlib')
if sessionlib_opt == 'xsmp'
  sm_dep = dependency('sm', version : '>= 1.2.0')
  ice_dep = dependency('ice', version : '>= 1.0.0')
  conf.set('USE_XSMP', 1)
elif sessionlib_opt == 'no'
  sm_dep = dependency('', required : false)
  ice_dep = dependency('', required : false)
  configure_args += '\'--with-sessionlib=no\''
endif

# TLS
tls_opt = get_option('tls')
if tls_opt == 'gnutls'
  tls_dep = dependency('gnutls', version : '>= 3.7.1')
  conf.set('USE_GNUTLS', 1)
elif tls_opt == 'openssl'
  tls_dep = dependency('openssl', version : '>= 1.1.1')
  conf.set('USE_OPENSSL', 1)
  configure_args += '\'--with-tls=openssl\''
endif

# migemo
migemo_dep = cpp_compiler.find_library('migemo', required : get_option('migemo'))
if migemo_dep.found() and cpp_compiler.has_header('migemo.h')
  conf.set('HAVE_MIGEMO_H', 1)
  configure_args += '\'--with-migemo\''
endif
migemodict = get_option('migemodict')
if migemodict != ''
  conf.set_quoted('MIGEMODICT', migemodict)
  configure_args += '\'--with-migemodict=@0@\''.format(migemodict)
  message('Default path for migemo dictionary file: @0@'.format(migemodict))
endif

# alsa
alsa_dep = dependency('alsa', version : '>= 1.0.0', required : get_option('alsa'))
if alsa_dep.found()
  conf.set('USE_ALSA', 1)
  configure_args += '\'--with-alsa\''
endif


# googletestが見つからない場合はテストはしない
build_tests_opt = get_option('build_tests')
if not build_tests_opt.disabled()
  # ディストロのパッケージとmeson wrapに対応
  gtest_main_dep = dependency('gtest_main',
                              fallback : ['gtest', 'gtest_main_dep'],
                              version : '>= 1.10.0',
                              required : build_tests_opt)
  build_tests = gtest_main_dep.found()
else
  build_tests = false
endif

#
# オプションの機能
#

# Use PangoLayout instead of PangoGlyphString
pangolayout = get_option('pangolayout').enabled()
if pangolayout
  conf.set('USE_PANGOLAYOUT', 1)
  configure_args += '\'--with-pangolayout\''
  message('Render text by PangoLayout: YES')
endif

# compatible cache directory
compat_cache_dir = get_option('compat_cache_dir').enabled()
if compat_cache_dir
  conf.set('ENABLE_COMPAT_CACHE_DIR', 1)
  message('Use compatible cache directory: YES')
else
  configure_args += '\'--disable-compat-cache-dir\''
  message('Use compatible cache directory: NO')
endif


#
# コンパイラーの追加オプション
#

# gprof support
gprof = get_option('gprof').enabled()
if gprof
  args = ['-pg']
  if cpp_compiler.has_multi_arguments(args)
    add_project_arguments(args, language: 'cpp')
    add_project_link_arguments(args,  language: 'cpp')
    configure_args += '\'--enable-gprof\''
  else
    error('not support -pg')
  endif
  message('Output profile information for gprof : YES')
endif

# CPUの最適化オプション
native = get_option('native').enabled()
if native
  args = ['-march=native']
  if cpp_compiler.has_multi_arguments(args)
    add_project_arguments(args, language : 'cpp')
    configure_args += '\'--with-native\''
  else
    error('not support -march=native')
  endif
  message('Optimize to your machine: YES')
endif


#
# ビルドの情報
#
conf.set('HAVE_BUILDINFO_H', 1)
packager_opt = get_option('packager')
if packager_opt != ''
  conf.set_quoted('PACKAGE_PACKAGER', packager_opt)
endif

if configure_args.length() > 0
  conf.set_quoted('CONFIGURE_ARGS', ' '.join(configure_args))
endif


subdir('src')
if build_tests
  subdir('test', if_found : gtest_main_dep)
endif

summary({'alsa' : alsa_dep.found(),
         'build_tests' :  build_tests,
         'compat_cache_dir' : compat_cache_dir,
         'gprof' : gprof,
         'migemo' : migemo_dep.found(),
         'migemodict' : migemodict,
         'native' : native,
         'packager' : packager_opt,
         'pangolayout' : pangolayout,
         'sessionlib' : sessionlib_opt,
         'tls' : tls_opt,
         }, section : 'Configuration', bool_yn : true)


#
# プログラムと一緒にインストールするアイコンや設定など
#
install_data('jdim.png', install_dir : get_option('datadir') / 'icons/hicolor/48x48/apps')
install_data('jdim.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/apps')

install_data('jdim.desktop', install_dir : get_option('datadir') / 'applications')
install_data('jdim.metainfo.xml', install_dir : get_option('datadir') / 'metainfo')
