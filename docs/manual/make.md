---
title: make、実行方法について
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [動作環境](#environment)
- [makeに必要なツール、ライブラリ](#requirement)
- [ビルド方法( meson )](#build-meson)
- [ビルドオプション](#build-option)
- [メモ](#memo)


<a name="environment"></a>
### 動作環境

#### 必須環境
- gtkmm 3.24.2 以上
- glibmm 2.64.2 以上
- glib 2.66.8 以上
- zlib 1.2 以上
- gnutls 3.7.1 以上

#### 推奨環境
- Linux kernel 5.10.0 以上
- UTF-8環境 ( EUC環境では `LANG="ja_JP.UTF-8"` を指定する必要がある )

※ GTK2版はv0.4.0リリースをもって廃止 ( <https://github.com/JDimproved/JDim/issues/229> を参照 )


<a name="requirement"></a>
### makeに必要なツール、ライブラリ

Autotools(./configure)のサポートはv0.10.0(2023年7月)のリリースをもって廃止されました。
かわりに[meson][meson]を利用してください。([RFC 0012][rfc0012])

#### 必須
- meson 0.61.0 以上
- g++ 10 以上、または clang++ 11 以上
- gnutls
- gtkmm
- zlib

#### オプション
- alsa-lib (`-Dalsa=enabled`)
- openssl 1.1.1 以上 (`-Dtls=openssl`)
- migemo (`-Dmigemo=enabled`)
- googletest 1.10.0 以上 ([test/RADME.md][testreadme]を参照)

#### WebPやAVIF画像の表示に必要なパッケージ
インストールされていない環境では`.webp`や`.avif`で終わるURLは通常リンクになる。
- libwebp, webp-pixbuf-loader (WebP)
- libavif (AVIF)

WebPやAVIF形式の画像を表示する方法は [#737][dis737] を参照。
<small>(v0.5.0+からサポート)</small>

OSやディストリビューション別の解説は [#592][dis592] を参照。



<a name="build-meson"></a>
### ビルド方法( meson )

1. `meson setup builddir`
2. `ninja -C builddir` ( または `meson compile -C builddir` )
3. 起動は `./builddir/src/jdim`
4. (お好みで) `strip ./builddir/src/jdim`

#### mesonのビルドオプション
- `meson setup builddir -Dpangolayout=enabled` のように指定する。
- オプションの一覧は `meson configure` を実行してProject optionsの段落を参照する。


<a name="build-option"></a>
### ビルドオプション
<dl>
  <dt>-Dsessionlib=[xsmp|no]</dt>
  <dd>
    XSMP を使ってセッション管理をするには <code>xsmp</code> を、
    セッション管理を無効にするには <code>no</code> を選択する。デフォルトでは XSMP を使用する。
  </dd>
  <dt>-Dpangolayout=enabled</dt>
  <dd>
    描画に PangoLayout を使う。デフォルトでは PangoGlyphString を使用する。
    スレビューのテキスト表示に問題があるときはこのオプションを試してみてください。<br>
    about:config 「スレビューのテキストを描画する方法 ( 0: PangoGlyphString 1: PangoLayout )」で変更が可能。
    変更後に開いたスレから適用される。 <small>(v0.10.1-20230930から追加)</small>
  </dd>
  <dt>-Dmigemo=enabled</dt>
  <dd>
    migemo による検索が有効になる。migemoがUTF-8の辞書でインストールされている必要がある。
    有効にすると正規表現のメタ文字が期待通りに動作しない場合があるので注意すること。
  </dd>
  <dt>-Dmigemodict=PATH</dt>
  <dd>
    (<code>-Dmigemo=enabled</code> 限定) migemo の辞書ファイルの場所を設定する。
    about:config で変更が可能、空欄にした場合は migemo が無効になる。(変更後は要再起動)
  </dd>
  <dt>-Dnative=enabled</dt>
  <dd>
    CPUに合わせた最適化。
    このオプションは<code>--buildtype=release</code>または<code>--optimization=3</code>などの最適化オプションと一緒に使います。
    通常は最適化オプションだけで十分な効果があるため、妥協できる場合は必要ありません。
  </dd>

  <dt>-Dtls=[gnutls|openssl]</dt>
  <dd>使用するSSL/TLSライブラリを設定する。デフォルトでは GnuTLS を使用する。</dd>
  <dt>-Dtls=openssl</dt>
  <dd>GnuTLS のかわりに OpenSSL を使用する。ライセンス上バイナリ配布が出来なくなることに注意すること。</dd>

  <dt>-Dalsa=enabled</dt>
  <dd>ALSA による効果音再生機能を有効にする。
  詳しくは<a href="{{ site.baseurl }}/sound/">効果音の再生</a>の項を参照すること。</dd>
  <dt>-Dgprof=enabled</dt>
  <dd>
    gprof によるプロファイリングを行う。
    コンパイルオプションに <code>-pg</code> が付き、JDimを実行すると <code>gmon.out</code> が出来るので
    <code>gprof  ./jdim  gmon.out</code> で解析できる。CPUの最適化は効かなくなるので注意する。
  </dd>

  <dt>-Dcompat_cache_dir=disabled</dt>
  <dd>JDのキャッシュディレクトリ <code>~/.jd</code> を読み込む互換機能を無効化する。</dd>
  <dt>-Dpackager=PACKAGER</dt>
  <dd>
    動作環境にパッケージや作成者の情報を追加する。<small>(v0.7.0+から追加)</small>
    <code>PACKAGER</code> に改行やHTML文字参照を <em>含めない</em> ことを推奨する。
  </dd>
</dl>


<a name="memo"></a>
### メモ
実行するには直接 `builddir/src/jdim` を起動するか手動で `/usr/bin` あたりに `builddir/src/jdim` を `cp` する。

以上の操作で ninja が通らなかったり動作が変な時は meson のビルドオプションを変更する。


[testreadme]: https://github.com/JDimproved/JDim/blob/master/test/README.md
[meson]: https://mesonbuild.com
[dis592]: https://github.com/JDimproved/JDim/discussions/592 "OS/ディストリビューション別インストール方法 - Discussions #592"
[dis737]: https://github.com/JDimproved/JDim/discussions/737 "[v0.5.0+] WebPやAVIF形式の画像を表示する方法 - Discussions #737"
[rfc0012]: https://github.com/JDimproved/rfcs/blob/master/docs/0012-end-of-autotools-support.md
