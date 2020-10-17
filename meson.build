# JDim用 meson.build
# NOTE: 実験的なサポートのため変更または廃止の可能性がある

# Autotoolsからの移行はGNOMEのガイドラインを参考にする
# https://wiki.gnome.org/Initiatives/GnomeGoals/MesonPorting

# mesonを使ってJDimをビルドする方法
#
# Fedora
#   ディストロのパッケージをインストールする
#   ```
#   dnf install git libtool meson ninja-build
#   dnf install gnutls-devel gtkmm30-devel libSM-devel
#   ```
#
# Debian Buster, Ubuntu 20.04
#   ディストロのパッケージをインストールする
#   ```
#   sudo apt install build-essential git libtool meson ninja-build
#   sudo apt install libgnutls28-dev libgtkmm-3.0-dev libltdl-dev
#   ```
#
# Debian Stretch, Ubuntu 18.04
#   ディストロのmesonパッケージはバージョンが古いためpypiからインストールする
#   $HOME/.local/bin にパスを通す
#   ```
#   sudo apt install build-essential git libtool ninja-build python3 python3-pip
#   pip3 install --user meson
#   sudo apt install libgnutls28-dev libgtkmm-3.0-dev libltdl-dev
#   ```

# ビルドの手順
#   ```
#   git clone -b master --depth 1 https://github.com/JDimproved/JDim.git jdim
#   cd jdim
#   meson builddir
#   cd builddir
#   ninja
#   ```
# Tips
#   - JDimのビルドオプションは `meson configure` で確認できる
#   - ビルドオプションは `meson builddir -Dregex=glib` のように指定する
#   - 生成された実行ファイルの場所は builddir/src/jdim

project('jdim', 'cpp',
        version : '0.4.0',
        license : 'GPL2',
        meson_version : '>= 0.49.2',
        default_options : ['warning_level=3', 'cpp_std=c++11'])

# 追加コンパイルオプション
add_project_arguments('-DHAVE_CONFIG_H=1', language : 'cpp')
add_project_arguments('-DGTK_DOMAIN="gtk30"', language : 'cpp')
# -Wextraで有効になる-Wunused-parameterは修正方法の検討が必要なので暫定的に無効
add_project_arguments('-Wno-unused-parameter', language : 'cpp')

cpp_compiler = meson.get_compiler('cpp')


conf = configuration_data()
# オプションの表示にはconfigureスタイル('--with-foo')が必要なので注意
configure_args = []

#
# OSを判定してOS別の設定
#
system = host_machine.system()
if system == 'freebsd'
  conf.set('USE_MKTIME', 1)
elif system == 'sunos'
  conf.set('NO_TIMEGM', 1)
elif system == 'darwin'
  conf.set('USE_MKTIME', 1)
endif


#
# 必須パッケージのチェック
#
gtkmm_dep = dependency('gtkmm-3.0', version : '>= 3.22.0', required : false)
if not gtkmm_dep.found()
  warning('JDim will require gtkmm-3.0 >= 3.22.0 in the future release.')
  gtkmm_dep = dependency('gtkmm-3.0', version : '>= 3.18.0')
endif
threads_dep = dependency('threads')
x11_dep = dependency('x11')
zlib_dep = dependency('zlib', version : '>= 1.2.0')


# crypt
crypt_dep = cpp_compiler.find_library('crypt', required : false)
if not crypt_dep.found()
  crypt_dep = dependency('libcrypt', required : false)
endif
if not crypt_dep.found()
  crypt_dep = dependency('libxcrypt')
endif
if cpp_compiler.has_header('crypt.h')
  conf.set('HAVE_CRYPT_H', 1)
endif

# socket
if cpp_compiler.has_header('sys/socket.h')
  socket_dep = dependency('', required : false)
else
  socket_dep = cpp_compiler.find_library('socket')
endif

# utsname
if cpp_compiler.has_header('sys/utsname.h')
  conf.set('HAVE_SYS_UTSNAME_H', 1)
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

# 正規表現ライブラリ
regex_opt = get_option('regex')
if regex_opt == 'posix'
  configure_args += '\'--with-regex=posix\''
  regex_dep = dependency('', required : false)
  conf.set('HAVE_REGEX_H', 1)
elif regex_opt == 'oniguruma'
  regex_dep = dependency('oniguruma')
  if not cpp_compiler.has_header('onigposix.h')
    error('onigposix.h not found')
  endif
  conf.set('HAVE_ONIGPOSIX_H', 1)
  configure_args += '\'--with-regex=oniguruma\''
elif regex_opt == 'glib'
  regex_dep = dependency('glib-2.0', version : '>= 2.14.0')
endif

# TLS
tls_opt = get_option('tls')
if tls_opt == 'gnutls'
  tls_dep = dependency('gnutls', version : '>= 3.4.10')
  conf.set('USE_GNUTLS', 1)
elif tls_opt == 'openssl'
  tls_dep = dependency('openssl', version : '>= 0.9.0')
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
  if cpp_compiler.has_header('alsa/asoundlib.h')
    conf.set('HAVE_ALSA_ASOUNDLIB_H', 1)
  endif
  configure_args += '\'--with-alsa\''
endif


# googletestが見つからない場合はテストはしない
# ディストロのパッケージとmeson wrapに対応
gtest_main_dep = dependency('gtest',
                            main : true,
                            fallback : ['gtest', 'gtest_main_dep'],
                            required : false)


#
# オプションの機能
#

# Use PangoLayout instead of PangoGlyphString
if get_option('pangolayout').enabled()
  conf.set('USE_PANGOLAYOUT', 1)
  configure_args += '\'--with-pangolayout\''
  message('Render text by PangoLayout: YES')
endif

# compatible cache directory
if get_option('compat_cache_dir').enabled()
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
if get_option('gprof').enabled()
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
if get_option('native').enabled()
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

buildinfo = configuration_data()

if configure_args.length() > 0
  conf.set_quoted('CONFIGURE_ARGS', ' '.join(configure_args))
endif


subdir('src')
subdir('test', if_found : gtest_main_dep)


#
# プログラムと一緒にインストールするアイコンや設定など
#
install_data('jdim.png', install_dir : get_option('datadir') / 'icons/hicolor/48x48/apps')
install_data('jdim.svg', install_dir : get_option('datadir') / 'icons/hicolor/scalable/apps')

install_data('jdim.desktop', install_dir : get_option('datadir') / 'applications')
install_data('jdim.metainfo.xml', install_dir : get_option('datadir') / 'metainfo')
