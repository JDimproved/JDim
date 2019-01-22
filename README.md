# JDim - 2ch browser for linux

ここに書かれていない詳細については[リポジトリ][repository]を参照してください。

[repository]: https://github.com/JDimproved/JDim

* [概要](#概要)
* [動作プラットフォーム](#動作プラットフォーム)
* [導入方法](#導入方法)
* [通常の起動](#通常の起動)
* [多重起動について](#多重起動について)
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


## 導入方法

### 事前準備

一度だけやればいい。

#### Redhat系
```sh
dnf install gtkmm24-devel gnutls-devel libSM-devel libtool automake git
```

#### Debian系
```sh
sudo apt-get build-dep jd
```

開発環境が入っていない場合は、

```sh
sudo apt-get install build-essential automake autoconf-archive git
```

#### Ubuntu 18.04
開発環境が入っていない場合は、

```sh
sudo apt-get install build-essential automake autoconf-archive git libtool
```

必要なライブラリを入れる。(抜けがあるかも)

```sh
sudo apt-get install libgtkmm-2.4-dev libmigemo1 libasound2-data libltdl-dev libasound2-dev libgnutls28-dev libgcrypt20-dev
```

### ビルド

```sh
git clone -b test --depth 1 https://github.com/JDimproved/JDim.git jdim
cd jdim
autoreconf -i
./configure
make
```

実行するには直接 src/jdim を起動するか手動で /usr/bin あたりに src/jdim を cp する。


### 参考

詳しいインストールの方法は [本家のwiki](https://ja.osdn.net/projects/jd4linux/wiki/OS%2f%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95) を参照。


### Tips

* **buildの高速化**

  make するときに `-j job数`(並列処理の数) を指定すれば高速にコンパイルできます。
  使用するCPUのコア数と相談して決めてください。

* **‘to_string’ is not a member of ‘std’なエラーが出た場合**

  このエラーが出た場合は、 automake のマクロが入っていないか、gcc のバージョンが古い可能性があります。
  automakeのマクロはubuntuでは `autoconf-archive` というパッケージ名です。

  またマクロを入れなくても、
  ```sh
  make CXXFLAGS+="-std=c++11"
  ```
  でも同様の効果があります。もしこれで駄目な場合はgccのversionが古すぎるので、
  gccのバージョンアップをするか、ディストリをバージョンアップしてください。

* **configureチェック中に `AX_CXX_COMPILE_STDCXX_11(noext, mandatory)` に関連したエラーがでた場合**

  ubuntuでは `autoconf-archive` をインストールして `autoreconf -i` からやり直してみてください。
  パッケージが見つからないまたはエラーが消えない場合は以下の手順を試してみてください。

  1. `configure.ac` の `AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])` の行を削除する。
  2. `autoreconf -i` で `configure` を作りconfigureチェックをやり直す。
  3. `make CXXFLAGS+="-std=c++11"` でビルドする。


## 通常の起動

使い方は以下のとおり。

```sh
$ jdim [OPTION] [URL,FILE]
```

引数にURLを付けて起動する事も出来るので、他のアプリケーションから外部コマンドとしてURLを開く事などが出来る。
(JDimが扱う事の出来るURLでない場合は設定されているWebブラウザに渡される)

```sh
$ jdim http://pc99.2ch.net/test/read.cgi/linux/1234567890/
```

ローカルにあるdatファイルを指定して、一時的にスレビュー表示させることも出来る。

```sh
$ jdim ./12345.dat
```

環境変数 `JD_CACHE` でキャッシュディレクトリの位置を変更・指定することが可能。
指定しなければ `~/.jd` がキャッシュディレクトリになる。

```sh
$ JD_CACHE=~/.mycache jdim
```

環境変数 `JD_LOCK` でロックファイルの位置を変更・指定することが可能。
指定しなければ `~/.jd/JDLOCK` がロックファイルになる。

```sh
$ JD_LOCK=~/mylock jdim
```

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

JDはメインプロセス/サブプロセスという関係で動作する。

* メインプロセス: 指令を受け取る事が出来るプロセス
* サブプロセス: 指令を出す事が出来るプロセス

通常は最初に起動した物がメインプロセスとなり、メインプロセスは1つだけ存在する事が出来る。
メインプロセスが存在する状態で起動したプロセスはサブプロセスとして扱われ、複数存在させる事も可能。
なお、指令を受け取るのはメインプロセスのみなので、指令を出す側のサブプロセスでURLは開かれない。

以下のコマンドを使い分ける事でサブプロセスの起動のしかたをコントロール出来る。

* a. 起動するかどうか確認してサブプロセスを起動
  ```sh
  $ jdim
  ```
* b. 確認せずにサブプロセスを起動
  ```sh
  $ jdim -m
  ```
* c. メインプロセスにURLを渡してサブプロセスを起動
  ```sh
  $ jdim -m http://pc99.2ch.net/test/read.cgi/linux/1234567890/
  ```

注: サブプロセスを残したままメインプロセスを終了していた場合は次に起動したプロセスがメインプロセスとなる。


## 著作権

© 2017-2019 yama-natuki [https://github.com/yama-natuki/JD]  
© 2019 JDimproved project [https://github.com/JDimproved/JDim]

パッチやファイルを取り込んだ場合、それらのコピーライトは「JDimproved project」に統一します。

fork元の JD:

© 2006-2015 JD project [https://ja.osdn.net/projects/jd4linux/]


## ライセンス

JDim 及びfork元の JD は GPLv2 の下で公開されています。
[GNU General Public License, version 2][lisence]

[lisence]: https://github.com/JDimproved/JDim/blob/master/COPYING


## 連絡先

バグ報告その他は [Linux板@５ちゃんねる](http://mao.5ch.net/linux/) のJD/JDimスレ、
またはJDimのリポジトリにて行ってください。
