# JDim - 2ch browser for linux

[![Build Status](https://travis-ci.com/JDimproved/JDim.svg?branch=master)](https://travis-ci.com/JDimproved/JDim)
![GitHub Actions CI](https://github.com/JDimproved/JDim/workflows/CI/badge.svg)
[![Snap Status](https://build.snapcraft.io/badge/JDimproved/JDim.svg)](https://build.snapcraft.io/user/JDimproved/JDim)
[![jdim](https://snapcraft.io//jdim/badge.svg)](https://snapcraft.io/jdim)

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
  * [GTK3版について](#GTK3版について)
  * [Snapパッケージ](#Snapパッケージ)
* [通常の起動](#通常の起動)
  * [コマンドライン オプション](#コマンドライン-オプション)
* [多重起動について](#多重起動について)
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
WindowsではMinGWを使ってビルド可能ですが、動作はまだ安定していないようです。

##### サポートの最新情報
GTK2版はサポートを終了する予定となりました。
Ubuntu 16.04 LTS(2016年)より前にリリースされたディストリビューションを利用されている場合は更新をお願いいたします。


## 導入方法

ソースコードからJDimをビルドします。**デフォルトの設定ではGTK3版がビルド**されますのでご注意ください。
詳細は [INSTALL](./INSTALL) にも書いてあります。


### 事前準備

ツールチェーンとライブラリをインストールします。一度インストールすれば次回から事前準備はいりません。

#### Redhat系
*GTK3版*
```sh
dnf install gtkmm30-devel gnutls-devel libSM-devel libtool automake autoconf-archive git
```

*GTK2版* - `gtkmm30-devel` のかわりに `gtkmm24-devel` をインストールします。
```sh
dnf install gtkmm24-devel gnutls-devel libSM-devel libtool automake autoconf-archive git
```

#### Debian (stretch-backportsあるいはbuster以降)
```sh
sudo apt install libc6-dev make gcc g++ git
sudo vi /etc/apt/sources.list
# ↑エディタは何でも良い。deb-src行でstretch-backportsあるいはbuster以降を有効にする
sudo apt update
sudo apt build-dep jdim
```

#### Ubuntu 18.04
開発環境が入っていない場合は、

```sh
sudo apt install build-essential automake autoconf-archive git libtool
```

必要なライブラリを入れます。(抜けがあるかも)

*GTK3版*
```sh
sudo apt install libgtkmm-3.0-dev libmigemo1 libasound2-data libltdl-dev libasound2-dev libgnutls28-dev
```

*GTK2版* - `libgtkmm-3.0-dev` のかわりに `libgtkmm-2.4-dev` をインストールします。
```sh
sudo apt install libgtkmm-2.4-dev libmigemo1 libasound2-data libltdl-dev libasound2-dev libgnutls28-dev
```


### ビルド

*GTK3版 (デフォルト)*
```sh
git clone -b master --depth 1 https://github.com/JDimproved/JDim.git jdim
cd jdim
autoreconf -i
./configure
make
```

*GTK2版* - ./configure にオプション `--with-gtkmm3=no` を追加します。
```sh
git clone -b master --depth 1 https://github.com/JDimproved/JDim.git jdim
cd jdim
autoreconf -i
./configure --with-gtkmm3=no
make
```

実行するには直接 src/jdim を起動するか手動で /usr/bin あたりに src/jdim を cp します。

#### Arch Linux
GTK3版のビルドファイルはAURで公開されています。(Thanks to @naniwaKun.)  
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
  ###### 例 (Intel Core 2)
  ```sh
  ./configure CXXFLAGS="-march=core2 -O2"
  ```

  マシンのCPUは下のコマンドで調べることができます。([GCCの最適化][gentoo-gcc] - Gentoo Wikiより)
  ```sh
  gcc -Q -c -march=native --help=target -o /dev/null | grep "march\|mtune\|mcpu"
  ```

* **configureチェック中に `AX_CXX_COMPILE_STDCXX_11(noext, mandatory)` に関連したエラーがでた場合**

  ubuntuでは `autoconf-archive` をインストールして `autoreconf -i` からやり直してみてください。
  パッケージが見つからないまたはエラーが消えない場合は以下の手順を試してみてください。

  1. `configure.ac` の `AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])` の行を削除する。
  2. `autoreconf -i` で `configure` を作りconfigureチェックをやり直す。
  3. `make CXXFLAGS+="-std=c++11"` でビルドする。

  もしこれで駄目な場合はgccのversionが古すぎるので、
  gccのバージョンアップをするか、ディストリをバージョンアップしてください。

[gentoo-gcc]: https://wiki.gentoo.org/wiki/GCC_optimization/ja#-march


### GTK3版について

GTK3版はGTK2版同様のルック・アンド・フィールになるように実装していますが、
技術的な問題やテスト不足から完全な再現はできていません。
もしお気づきの点などがございましたらご指摘いただけると幸いです。

#### マウスホイール操作
~~板一覧やスレ一覧でマウスホイールによるスクロールが動作しないことがあります。
環境変数 `GDK_CORE_DEVICE_EVENTS=1` を設定してjdimを起動するとマウスホイール機能が使えます。~~
**0.2.0-20190827以降のバージョンで修正されています。**
```sh
# シェルからJDimを起動する場合
GDK_CORE_DEVICE_EVENTS=1 ./src/jdim
```

#### GTK2版から変更/追加された部分
* GTK+ 3.14以上の環境でタッチスクリーンによる操作に対応した。
  スレビューのタッチ操作については[マニュアル][manual-touch]を参照。
* 書き込みビューの配色にGTKテーマを使う設定が追加された。
  1. メニューバーの`設定(C) > フォントと色(F) > 詳細設定(R)...`からフォントと色の詳細設定を開く
  2. `色の設定`タブにある`書き込みビューの配色設定に GTKテーマ を用いる(W)`をチェックして適用する
* GTK 3.16以上の環境で書き込みビューのダブルクリックによる単語の範囲選択、
  トリプルクリックによる行の範囲選択に対応した。

[manual-touch]: https://jdimproved.github.io/JDim/operation/#threadview_touch "操作方法について | JDim"

#### GTK3版の既知の問題
* ~~マウスホイールでタブを切り替える機能が動作しない環境がある。(gtk 3.20以上?)~~
  **0.2.0-20190827以降のバージョンで修正されています。**
* タブのドラッグ・アンド・ドロップの矢印ポップアップの背景が透過しない環境がある。
  (アルファチャンネルが利用できない環境)
* ~~スレビュー表示内にアスキーアート(AA)と判定されたレスがあると、スレビュー全体にAAのフォント設定が適用される。
  この問題はオプション `--with-pangolayout` を追加してビルドすると回避できる。~~
  **0.2.0-20190921以降のバージョンで修正されています。**
* ~~お気に入りにマウスオーバーすると表示されるツールチップがポインターからずれる。~~
  **0.2.0-20191005以降のバージョンで修正されています。**
* ~~メニューから各部のフォント設定を選択するとクラッシュする。~~
* ~~ダイアログウインドウの幅が項目の長さに合わせて大きくなり縮小できない。~~
* ~~スレビューのスクロールバーがスクロール操作や検索したレスへの移動に反応せず、その後も1操作遅れて反応する。~~<br>
  **上記の３件は0.2.0-20191019以降のバージョンで修正されています。**
* ~~マウスジェスチャー（右クリック）中にマウスホイール操作してもタブを切り替える機能が動作しない。~~
  **0.3.0-20200308以降のバージョンで修正された。**


### Snapパッケージ
JDim GTK3版はSnapパッケージとして[Snap Storeで公開][snapcraft]されています。
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

### コマンドライン オプション

オプション | 説明
--- | ---
-h, --help | ヘルプを表示
-m, --multi | 多重起動時のサブプロセスであっても終了しない
-n, --norestore | 前回異常終了した時にバックアップファイルを復元しない
-s, --skip-setup | 初回起動時の設定ダイアログを表示しない
-l, --logfile | エラーなどのメッセージをファイル(キャッシュディレクトリのlog/msglog)に出力する
-g, --geometry WxH-X+Y | 幅(W)高さ(H)横位置(X)縦位置(Y)の指定。WxHは省略化(例: -g 100x40-10+30, -g -20+100 )
-V, --version | バージョン及びconfigureオプションを全て表示


## 多重起動について

[マニュアル][manual-multiple]を参照してください。

[manual-multiple]: https://jdimproved.github.io/JDim/start/#multiple "起動について | JDim"


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
© 2019-2020 JDimproved project [https://github.com/JDimproved/JDim]

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
