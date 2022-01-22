# JDim - 2ch browser for linux

![GitHub Actions CI](https://github.com/JDimproved/JDim/workflows/CI/badge.svg)
[![Snap Store](https://snapcraft.io/jdim/badge.svg)](https://snapcraft.io/jdim)

ここに書かれていない詳細は [オンラインマニュアル][manual] を、
開発に参加するための手順については [CONTRIBUTING][contrib] や [RFC][rfcs] を参照してください。

[manual]: https://jdimproved.github.io/JDim/
[repository]: https://github.com/JDimproved/JDim
[rfcs]: https://github.com/JDimproved/rfcs "Request for Comments"

* [概要](#概要)
* [動作プラットフォーム](#動作プラットフォーム)
* [導入方法](#導入方法)
  * [事前準備](#事前準備)
  * [ビルド](#ビルド)
  * [Snapパッケージ](#Snapパッケージ)
* [通常の起動](#通常の起動)
  * [コマンドライン オプション](#コマンドライン-オプション)
* [多重起動について](#多重起動について)
* [実行時の注意事項](#precaution)
* [JDとの互換性](#JDとの互換性)
  * [JDから移行する](#JDから移行する)
* [著作権](#著作権)
* [ライセンス](#ライセンス)
* [連絡先](#連絡先)


## 概要

JDim (JD improved) は gtkmm/GTK+ を使用した"２ちゃんねる"型マルチスレッドBBSを閲覧するためのブラウザです。
JDim は GPLv2 の下で公開されている [JD][jd-project] からforkしたソフトウェアであり、
ルック・アンド・フィールや環境設定は JD と互換性があります。

**注意: JDim本体は5ch.netのAPIに対応しておりません。**
ご不便をおかけして申し訳ありませんが、5ch.netにアクセスする場合はWebブラウザなどをご使用ください。

[jd-project]: https://jd4linux.osdn.jp/


## 動作プラットフォーム

LinuxなどのUnixライクなOS(FreeBSD,OpenBSD,Nexenta,MacOSXでも動作報告例があります)。

##### サポートの最新情報
gccのバージョンが7未満のプラットフォームはサポートを終了しました。
Ubuntu 18.04(2018年)より前にリリースされたディストリビューションを利用されている場合は更新をお願いいたします。

メンテナンスの都合によりWindows(MinGW)版のサポートは[終了][#445]しました。

[#445]: https://github.com/JDimproved/JDim/issues/445


## 導入方法

ソースコードからJDimをビルドします。**GTK3版がビルド**されますのでご注意ください。
詳細は [INSTALL](./INSTALL) にも書いてあります。

ビルドツール [meson][mesonbuild] を暫定的にサポートしています。
configure のかわりに meson を使ってビルドする方法は [Discussions][dis556] を参照してください。

[mesonbuild]: https://mesonbuild.com
[dis556]: https://github.com/JDimproved/JDim/discussions/556 "Mesonを使ってJDimをビルドする方法 - Discussions #556"


### 事前準備

ツールチェーンとライブラリをインストールします。一度インストールすれば次回から事前準備はいりません。

#### Redhat系
```sh
dnf install gtkmm30-devel gnutls-devel libSM-devel libtool automake autoconf-archive git
```

#### Debian (buster以降)
```sh
sudo apt install libc6-dev make gcc g++ git
sudo vi /etc/apt/sources.list
# ↑エディタは何でも良い。deb-src行でbuster以降を有効にする
sudo apt update
sudo apt build-dep jdim
```

#### Ubuntu (18.04以降)
開発環境が入っていない場合は、

```sh
sudo apt install build-essential automake autoconf-archive git libtool
```

必要なライブラリを入れます。(抜けがあるかも)

```sh
sudo apt install libgtkmm-3.0-dev libmigemo1 libasound2-data libltdl-dev libasound2-dev libgnutls28-dev
```


### ビルド

```sh
git clone -b master --depth 1 https://github.com/JDimproved/JDim.git jdim
cd jdim
autoreconf -i
./configure
make
```

実行するには直接 src/jdim を起動するか手動で /usr/bin あたりに src/jdim を cp します。

#### Arch Linux
ビルドファイルはAURで公開されています。(Thanks to @naniwaKun.)  
https://aur.archlinux.org/packages/jdim-git/

AUR Helper [yay](https://github.com/Jguer/yay) でインストール
```sh
yay -S jdim-git
```


### 参考

詳しいインストールの方法は [本家のwiki](https://ja.osdn.net/projects/jd4linux/wiki/OS%2f%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95) を参照してください。


### Tips

* **buildの高速化**

  make するときに `-j job数`(並列処理の数) を指定すれば高速にコンパイルできます。
  使用するCPUのコア数と相談して決めてください。

* **CPUに合わせた最適化**

  `./configure`を実行するときにCPUの種類(`-march=ARCH`や`-mcpu=CPU`)と最適化レベル(`-O`)を`CXXFLAGS`に設定します。
  ###### 例 (第2世代Coreプロセッサー)
  ```sh
  ./configure CXXFLAGS="-march=sandybridge -O2"
  ```

  マシンのCPUは下のコマンドで調べることができます。([GCCの最適化][gentoo-gcc] - Gentoo Wikiより)
  ```sh
  gcc -Q -c -march=native --help=target -o /dev/null | grep "march\|mtune\|mcpu"
  ```

* **configureチェック中に `AX_CXX_COMPILE_STDCXX(17, noext, mandatory)` に関連したエラーがでた場合**

  ubuntuでは `autoconf-archive` をインストールして `autoreconf -i` からやり直してみてください。
  パッケージが見つからないまたはエラーが消えない場合は以下の手順を試してみてください。
  または`./configure`のかわりにMesonを利用してビルドする方法があります。([GitHub][dis556]を参照)

  1. `configure.ac` の `AX_CXX_COMPILE_STDCXX([17], [noext], [mandatory])` の行を削除する。
  2. `autoreconf -i` で `configure` を作りconfigureチェックをやり直す。
  3. `make CXXFLAGS+="-std=c++17"` でビルドする。

  もしこれで駄目な場合はgccのversionが古すぎるので、
  gccのバージョンアップをするか、ディストリをバージョンアップしてください。

[gentoo-gcc]: https://wiki.gentoo.org/wiki/GCC_optimization/ja#-march


### Snapパッケージ
JDim はSnapパッケージとして[Snap Storeで公開][snapcraft]されています。
詳細は[マニュアル][manual-snap]を参照してください。

[snapcraft]: https://snapcraft.io/jdim
[manual-snap]: https://jdimproved.github.io/JDim/start#snap


## 通常の起動

使い方は以下のとおりです。

```sh
$ jdim [OPTION] [URL,FILE]
```

引数にURLを付けて起動する事も出来るので、他のアプリケーションから外部コマンドとしてURLを開く事などが出来ます。
(JDimが扱う事の出来るURLでない場合は設定されているWebブラウザに渡されます)

```sh
$ jdim http://pc99.2ch.net/test/read.cgi/linux/1234567890/
```

ローカルにあるdatファイルを指定して、一時的にスレビュー表示させることも出来ます。

```sh
$ jdim ./12345.dat
```

環境変数 `JDIM_CACHE` でキャッシュディレクトリの位置を変更・指定することが可能です。
指定しなければ下記の[優先順位](#キャッシュディレクトリの優先順位)の通りに決まります。

```sh
$ JDIM_CACHE=~/.mycache jdim
```

環境変数 `JDIM_LOCK` でロックファイルの位置を変更・指定することが可能です。
指定しなければ `<キャッシュディレクトリ>/JDLOCK` がロックファイルになります。

```sh
$ JDIM_LOCK=~/mylock jdim
```

#### キャッシュディレクトリの優先順位
| `~/.jd` | `$XDG_CACHE_HOME/jdim` | 使われるのは… |
| --- | --- | --- |
| 存在する | any | `~/.jd` |
| 存在しない | any | `$XDG_CACHE_HOME/jdim` |
| any (無効化) | any | `$XDG_CACHE_HOME/jdim` |

NOTE:

- 環境変数 `XDG_CACHE_HOME` が未設定または空のときはかわりに `$HOME/.cache/jdim` が使われます。
- `~/.jd` が無効化されている場合は `jdim --version` の出力に `--disable-compat-cache-dir` が追加されます。
- キャッシュディレクトリはJDimを起動すると作成されます。

### コマンドライン オプション

オプション | 説明
--- | ---
-h, --help | ヘルプを表示
-m, --multi | 多重起動時のサブプロセスであっても終了しない
-s, --skip-setup | 初回起動時の設定ダイアログを表示しない
-l, --logfile | エラーなどのメッセージをファイル(キャッシュディレクトリのlog/msglog)に出力する
-g, --geometry WxH-X+Y | 幅(W)高さ(H)横位置(X)縦位置(Y)の指定。WxHは省略可能(例: -g 100x40-10+30, -g -20+100 )
-V, --version | バージョン及びconfigureオプションを全て表示


## 多重起動について

[マニュアル][manual-multiple]を参照してください。

[manual-multiple]: https://jdimproved.github.io/JDim/start/#multiple "起動について | JDim"


<a name="precaution"></a>
## 実行時の注意事項

廃止されたGTK2版同様のルック・アンド・フィールになるように実装していますが、
技術的な問題やテスト不足から完全な再現はできていません。
もしお気づきの点などがございましたらご指摘いただけると幸いです。

### Wayland対応
JDim はWayland環境で起動しますが動作は安定していません。
GTKのバックエンドにWaylandを使うかわりに互換レイヤーのXWaylandをインストールして使うことをお薦めします。
環境変数 `GDK_BACKEND=x11` を設定してjdimを起動してください。
```sh
# シェルからJDimを起動する場合
GDK_BACKEND=x11 ./src/jdim
```

WaylandやXWaylandではX11限定の機能を使うことができないため注意してください。

### GTK2版から変更/追加された部分
* GTK+ 3.14以上の環境でタッチスクリーンによる操作に対応した。
  スレビューのタッチ操作については[マニュアル][manual-touch]を参照。
* 書き込みビューの配色にGTKテーマを使う設定が追加された。
  1. メニューバーの`設定(C) > フォントと色(F) > 詳細設定(R)...`からフォントと色の詳細設定を開く
  2. `色の設定`タブにある`書き込みビューの配色設定に GTKテーマ を用いる(W)`をチェックして適用する
* GTK 3.16以上の環境で書き込みビューのダブルクリックによる単語の範囲選択、
  トリプルクリックによる行の範囲選択に対応した。

[manual-touch]: https://jdimproved.github.io/JDim/operation/#threadview_touch "操作方法について | JDim"

### 既知の問題
* タブのドラッグ・アンド・ドロップの矢印ポップアップの背景が透過しない環境がある。
  (アルファチャンネルが利用できない環境)
* Wayland上で起動したときポップアップ内のアンカーからポップアップを出すとマウスポインターから離れた位置に表示される。
* Weston(Waylandコンポジタ)環境でXWaylandをバックエンドに指定して起動した場合、右クリックしながらポップアップ内に
  マウスポインターを動かすとポップアップ内容ではなくポップアップに隠れたスレビューに反応する。
* 32bit OSの古いMATE環境（バージョン[1.10][mate-1-10]から[1.16][mate-1-16]？）でJDim GTK3版を実行したとき
  スレビューの上に別のウインドウを重ねて移動させると残像でスレビュー内のみ描画が乱れる。
  スレビューをスクロール等させて再描画すると直る。([背景事情][mate-background])
* Wayland環境では画像ビュー(ウインドウ表示)のフォーカスが外れたら折りたたむ機能が正常に動作しない。

[mate-1-10]: https://mate-desktop.org/blog/2015-06-11-mate-1-10-released/ "GTK3の実験的なサポート追加"
[mate-1-16]: https://mate-desktop.org/blog/2016-09-21-mate-1-16-released/ "GTK3に移行途中"
[mate-background]: https://github.com/JDimproved/JDim/commit/ffbce60ede#commitcomment-40911816 "別の不具合が再発する"


## JDとの互換性

JDimの環境設定はJDからフォーマットを継承しているので後方互換性があります。
また、ユーザーインタフェースの変更は今のところありません。
JDimで追加された不具合や機能の修正については[Pull requests][pr-merged]を見てください。

[pr-merged]: https://github.com/JDimproved/JDim/pulls?q=is%3Apr+is%3Amerged


### JDから移行する

* 後方互換性としてJDのキャッシュディレクトリ(`~/.jd`)はそのまま使うことができます。
  ただし、オプション`--disable-compat-cache-dir`が指定されたビルドでは互換機能は無効化されます。
* 互換機能が使えないときは `$XDG_CACHE_HOME/jdim`(`~/.cache/jdim`) にキャッシュディレクトリを移動してください。
  ```bash
  $ mv ~/.jd ~/.cache/jdim
  ```
* 環境変数`JD_CACHE`でキャッシュディレクトリを設定している場合はかわりに`JDIM_CACHE`を使用してください。
* JDとJDimを併存させる(データや設定を共有しない)ためには環境変数によるキャッシュディレクトリの設定が必要です。
  ([通常の起動](#通常の起動)を参照)


## 著作権

© 2017-2019 yama-natuki [https://github.com/yama-natuki/JD]  
© 2019-2022 JDimproved project [https://github.com/JDimproved/JDim]

パッチやファイルを取り込んだ場合、それらのコピーライトは「JDimproved project」に統一します。

fork元の JD:

© 2006-2015 JD project [https://ja.osdn.net/projects/jd4linux/]


## ライセンス

JDim は GPLv2 の下で公開されています。
[GNU General Public License, version 2][lisence]

[lisence]: https://github.com/JDimproved/JDim/blob/master/COPYING


## 連絡先

バグ報告その他は [Linux板@５ちゃんねる](http://mao.5ch.net/linux/) のJD/JDimスレ、
またはJDimのリポジトリにて行ってください。詳しい方法は[ガイド][contrib]をご覧ください。

[contrib]: https://github.com/JDimproved/JDim/tree/master/CONTRIBUTING.md
