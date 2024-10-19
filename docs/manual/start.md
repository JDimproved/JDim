---
title: 起動について
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [通常の起動](#run)
- [多重起動について](#multiple)
- [実行時の注意事項](#precaution)
- [JDとの互換性](#compatibility)
- [Snapパッケージ](#snap)


<a name="run"></a>
### 通常の起動
使い方は以下のとおり。
```
$ jdim [OPTION] [URL,FILE]
```

引数にURLを付けて起動する事も出来るので、他のアプリケーションから外部
コマンドとしてURLを開く事などが出来る。(JDimが扱う事の出来るURLでない場
合は設定されているWebブラウザに渡される)
```
$ jdim http://pc99.2ch.net/test/read.cgi/linux/1234567890/
```

ローカルにあるdatファイルを指定して、一時的にスレビュー表示させることも出来る。
```
$ jdim ./12345.dat
```

環境変数 `JDIM_CACHE` でキャッシュディレクトリの位置を変更・指定することが可能。
指定しなければ下記の[優先順位](#cachepriority)の通りに決まる。
```
$ JDIM_CACHE=~/.mycache jdim
```

環境変数 `JDIM_LOCK` でロックファイルの位置を変更・指定することが可能。
指定しなければ `<キャッシュディレクトリ>/JDLOCK` がロックファイルになる。
```
$ JDIM_LOCK=~/mylock jdim
```

<a name="cachepriority"></a>
#### キャッシュディレクトリの優先順位

| `~/.jd` | `$XDG_CACHE_HOME/jdim` | 使われるのは… |
| --- | --- | --- |
| 存在する | any | `~/.jd` |
| 存在しない | any | `$XDG_CACHE_HOME/jdim` |
| any (無効化) | any | `$XDG_CACHE_HOME/jdim` |

NOTE:

- 環境変数 `XDG_CACHE_HOME` が未設定または空のときはかわりに `$HOME/.cache/jdim` が使われる。
- `~/.jd` が無効化されている場合は `jdim --version` の出力に `--disable-compat-cache-dir` が追加される。
- キャッシュディレクトリはJDimを起動すると作成される。

#### オプション
<dl>
  <dt>-h, --help</dt><dd>ヘルプを表示</dd>
  <dt>-m, --multi</dt><dd>多重起動時のサブプロセスであっても終了しない</dd>
  <dt>-s, --skip-setup</dt><dd>初回起動時の設定ダイアログを表示しない</dd>
  <dt>-l, --logfile</dt>
  <dd>エラーなどのメッセージをファイル(キャッシュディレクトリの<code>log/msglog</code>)に出力する</dd>
  <dt>-g, --geometry WxH-X+Y</dt>
  <dd>幅(W)高さ(H)横位置(X)縦位置(Y)の指定。
  WxHは省略可能(例: <code>-g 100x40-10+30</code>, <code>-g -20+100</code> )
  <br>注意: Wayland環境で起動したときは位置を指定しても反映されません。</dd>
  <dt>-V, --version</dt><dd>バージョン及びビルドオプションを全て表示</dd>
</dl>


<a name="multiple"></a>
### 多重起動について
JDimはメインプロセス/サブプロセスという関係で動作する。

- メインプロセス: 指令を受け取る事が出来るプロセス
- サブプロセス　: 指令を出す事が出来るプロセス

通常は最初に起動した物がメインプロセスとなり、メインプロセスは1つだけ存
在する事が出来る。メインプロセスが存在する状態で起動したプロセスはサブ
プロセスとして扱われ、複数存在させる事も可能。なお、指令を受け取るのは
メインプロセスのみなので、指令を出す側のサブプロセスでURLは開かれない。

以下のコマンドを使い分ける事でサブプロセスの起動のしかたをコントロール出来る。

- `-m`を指定しないときはメインプロセスのウインドウを最前面に表示する <small>(v0.11.0-20240330 から変更)</small><br>
  デスクトップ環境によって挙動が異なる可能性がある
  ```
  $ jdim
  ```
- 確認せずにサブプロセスを起動
  ```
  $ jdim -m
  ```
- メインプロセスにURLを渡してサブプロセスを起動
  ```
  $ jdim -m http://pc99.2ch.net/test/read.cgi/linux/1234567890/
  ```

注: サブプロセスを残したままメインプロセスを終了していた場合は次に起動
したプロセスがメインプロセスとなる。


<a name="precaution"></a>
### 実行時の注意事項

#### Wayland対応
JDim はWayland環境で起動するが動作は安定していない。
GTKのバックエンドにWaylandを使うかわりに互換レイヤーのXWaylandをインストールして使う。
環境変数 `GDK_BACKEND=x11` を設定してjdimを起動する。
```sh
# シェルからJDimを起動する場合
GDK_BACKEND=x11 ./src/jdim
```

WaylandやXWaylandではX11限定の機能を使うことができないため注意すること。

* Wayland環境では about:config の「自前でウィンドウ配置を管理する」の設定に関係なく、
  ウインドウ（メイン、書き込みビュー、画像ビュー）の位置を復元しません。([Issue #1450][#1450]を参照)
* Waylandでは、ウインドウ最大化を解除したときのウインドウのサイズは、最大化する前のサイズと異なる可能性があります。
  ([Issue #1463][#1463])

[#1450]: https://github.com/JDimproved/JDim/issues/1450
[#1463]: https://github.com/JDimproved/JDim/issues/1463

#### 既知の問題
* タブのドラッグ・アンド・ドロップの矢印ポップアップの背景が透過しない環境がある。
  (アルファチャンネルが利用できない環境)
* Wayland上で起動したときポップアップ内のアンカーからポップアップを出すとマウスポインターから離れた位置に表示される。
* Weston(Waylandコンポジタ)環境でXWaylandをバックエンドに指定して起動した場合、右クリックしながらポップアップ内に
  マウスポインターを動かすとポップアップ内容ではなくポップアップに隠れたスレビューに反応する。
* Wayland環境では画像ビュー(ウインドウ表示)のフォーカスが外れたら折りたたむ機能が正常に動作しない。
* gcc(バージョン10から13まで)を使いAddressSanitizer(ASan)を有効にしてビルドすると
  書き込みのプレビューでトリップを表示するときにクラッシュすることがある。([Issue #943][#943]を参照)
* Ubuntu 22.04(23.10でも確認)の環境でASanを有効にしてビルドしたプログラムを実行すると
  `AddressSanitizer:DEADLYSIGNAL`を出力し続けてハングアップすることがある。([README.md][readme-hungup-asan]を参照)


[#943]: https://github.com/JDimproved/JDim/issues/943
[readme-hungup-asan]: https://github.com/JDimproved/JDim/blob/master/README.md#hungup-with-asan


<a name="compatibility"></a>
### JDとの互換性
JDimの環境設定はJDからフォーマットを継承しているので後方互換性がある。
また、ユーザーインタフェースは今のところ変更されていない。
JDimで追加された不具合や機能の修正については[Pull requests][pr-merged]から見ることができる。

#### JDから移行する
* 後方互換性としてJDのキャッシュディレクトリ(`~/.jd`)はそのまま使うことができる。
  ただし、オプション`--disable-compat-cache-dir`が指定されたビルドでは互換機能は無効化される。
* 互換機能が使えないときは `$XDG_CACHE_HOME/jdim`(`~/.cache/jdim`) にキャッシュディレクトリを移動する。
  ```bash
  $ mv ~/.jd ~/.cache/jdim
  ```
* 環境変数`JD_CACHE`でキャッシュディレクトリを設定している場合はかわりに`JDIM_CACHE`を使用する。
* JDとJDimを併存させる(データや設定を共有しない)ためには環境変数によるキャッシュディレクトリの設定が要る。
  ([通常の起動](#run)を参照)


<a name="snap"></a>
### Snapパッケージ
JDim はSnapパッケージとして[Snap Storeで公開][snapcraft]されている。
`snap`コマンドやwebページからインストールすることでコマンドやデスクトップ環境のメニューから起動できる。

```sh
sudo snap install jdim
```

Snapパッケージ版はアクセス制限が導入されているため通常のパッケージやビルドと異なる点がある。

- JDimを起動したとき作成されるキャッシュディレクトリの場所が異なる (`~/snap/jdim/common/.cache/jdim/`)
- **上記Snap版のキャッシュディレクトリはパッケージを削除(アンインストール)すると消去される**
- JDのキャッシュディレクトリ (`~/.jd`) は使えない
- 隠しファイル（ドットファイル）のキャッシュディレクトリは使えない
- 外部コマンドの呼び出しは制限される
- 環境によってはGTKテーマ、アイコン、マウスカーソルがうまく表示されない場合がある<br>
  環境変数 (`GTK_THEME`) やGTKの設定ファイル (`$XDG_CONFIG_HOME/gtk-3.0/settings.ini`)
  を調整することで改善できるかもしれない

#### サポートの最新情報
Snap i386(32bit)版は2023年1月のリリースをもって[更新を終了][#890]した。
更新終了後も最後のバージョンは利用可能だがStoreからパッケージが削除される可能性がある。

[pr-merged]: https://github.com/JDimproved/JDim/pulls?q=is%3Apr+is%3Amerged
[snapcraft]: https://snapcraft.io/jdim
[#890]: https://github.com/JDimproved/JDim/issues/890

[操作方法について]: {{ site.baseurl }}/operation/#threadview_touch "操作方法について \| JDim"
