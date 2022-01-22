---
title: make、実行方法について
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [動作環境](#environment)
- [makeに必要なツール、ライブラリ](#requirement)
- [ビルド方法( configure + make の場合 )](#make-configure)
- [ビルド方法( meson の場合 )](#build-meson)
- [configureオプション](#configure-option)
- [メモ](#memo)


<a name="environment"></a>
### 動作環境

#### 必須環境
- gtkmm 3.22.0 以上
- glibmm 2.56.0 以上
- zlib 1.2 以上
- gnutls 3.5.18 以上

#### 推奨環境
- Linux Kernel 4.4.0 以上
- gtkmm 3.24.0 以上
- UTF-8環境 ( EUC環境では `LANG="ja_JP.UTF-8"` を指定する必要がある )

※ GTK2版はv0.4.0リリースをもって廃止 ( <https://github.com/JDimproved/JDim/issues/229> を参照 )


<a name="requirement"></a>
### makeに必要なツール、ライブラリ

#### 必須
- autoconf
- autoconf-archive
- automake
- g++ 7 以上、または clang++ 6.0 以上
- gnutls
- gtkmm
- libtool
- make
- zlib

#### オプション
- meson 0.49.0 以上
- alsa-lib (`--with-alsa`)
- openssl 1.1.0 以上 (`--with-tls=openssl`)
- migemo (`--with-migemo`)
- googletest ([test/RADME.md][testreadme]を参照)

#### 画像表示に必要なパッケージ
インストールされていない環境では`.webp`や`.avif`で終わるURLは通常リンクになる。
- libwebp, webp-pixbuf-loader (WebP)
- libavif (AVIF)

OSやディストリビューション別の解説は [#592][dis592] を参照。

configure のかわりに [meson] を使ってビルドする方法は [#556][dis556] を参照。
<small>(v0.4.0+から暫定サポート)</small>

WebPやAVIF形式の画像を表示する方法は [#737][dis737] を参照。
<small>(v0.5.0+からサポート)</small>


<a name="make-configure"></a>
### ビルド方法( configure + make の場合 )

1. `autoreconf -i` ( 又は `./autogen.sh` )
2. `./configure`
3. `make`
4. (お好みで) `strip src/jdim`


<a name="build-meson"></a>
### ビルド方法( meson の場合 )

1. `meson builddir`
2. `meson compile -C builddir` ( 又は `ninja -C builddir` )
3. 起動は `./builddir/src/jdim`

#### mesonのビルドオプション
- `meson builddir -Dpangolayout=enabled` のように指定する。
- オプションの一覧は `meson configure` を実行してProject optionsの段落を参照する。


<a name="configure-option"></a>
### configureオプション
<dl>
  <dt>--with-sessionlib=xsmp|no</dt>
  <dd>
    XSMP を使ってセッション管理をするには <code>xsmp</code> を、
    セッション管理を無効にするには <code>no</code> を選択する。デフォルトでは XSMP を使用する。
  </dd>
  <dt>--with-pangolayout</dt>
  <dd>
    描画に PangoLayout を使う。デフォルトでは PangoGlyphString を使用する。
    スレビューのテキスト表示に問題があるときはこのオプションを試してみてください。
  </dd>
  <dt>--with-migemo</dt>
  <dd>
    migemo による検索が有効になる。migemoがUTF-8の辞書でインストールされている必要がある。
    有効にすると正規表現のメタ文字が期待通りに動作しない場合があるので注意すること。
  </dd>
  <dt>--with-migemodict=PATH</dt>
  <dd>
    (<code>--with-migemo</code> 限定) migemo の辞書ファイルの場所を設定する。
    about:config で変更が可能、空欄にした場合は migemo が無効になる。(変更後は要再起動)
  </dd>
  <dt>--with-native</dt>
  <dd>CPUに合わせた最適化。CPUを指定する場合は <code>./configure CXXFLAGS="-march=ARCH"</code> を利用する。</dd>

  <dt>--with-tls=[gnutls|openssl]</dt>
  <dd>使用するSSL/TLSライブラリを設定する。デフォルトでは GnuTLS を使用する。</dd>
  <dt>--with-tls=openssl</dt>
  <dd>GnuTLS のかわりに OpenSSL を使用する。ライセンス上バイナリ配布が出来なくなることに注意すること。</dd>

  <dt>--with-alsa</dt>
  <dd>ALSA による効果音再生機能を有効にする。
  詳しくは<a href="{{ site.baseurl }}/sound/">効果音の再生</a>の項を参照すること。</dd>
  <dt>--enable-gprof</dt>
  <dd>
    gprof によるプロファイリングを行う。
    コンパイルオプションに <code>-pg</code> が付き、JDimを実行すると <code>gmon.out</code> が出来るので
    <code>gprof  ./jdim  gmon.out</code> で解析できる。CPUの最適化は効かなくなるので注意する。
  </dd>

  <dt>--disable-compat-cache-dir</dt>
  <dd>JDのキャッシュディレクトリ <code>~/.jd</code> を読み込む互換機能を無効化する。</dd>
</dl>


<a name="memo"></a>
### メモ
最近のディストリビューションの場合は `autogen.sh` よりも `autoreconf -i` の方を推奨。

実行するには直接 `src/jdim` を起動するか手動で `/usr/bin` あたりに `src/jdim` を `cp` する。

以上の操作でmakeが通らなかったり動作が変な時は configure のオプションを変更する。


[testreadme]: https://github.com/JDimproved/JDim/blob/master/test/README.md
[meson]: https://mesonbuild.com
[dis556]: https://github.com/JDimproved/JDim/discussions/556 "Mesonを使ってJDimをビルドする方法 - Discussions #556"
[dis592]: https://github.com/JDimproved/JDim/discussions/592 "OS/ディストリビューション別インストール方法 - Discussions #592"
[dis737]: https://github.com/JDimproved/JDim/discussions/737 "[v0.5.0+] WebPやAVIF形式の画像を表示する方法 - Discussions #737"
