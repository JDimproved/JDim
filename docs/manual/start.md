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

#### オプション
<dl>
  <dt>-h, --help</dt><dd>ヘルプを表示</dd>
  <dt>-m, --multi</dt><dd>多重起動時のサブプロセスであっても終了しない</dd>
  <dt>-n, --norestore</dt><dd>前回異常終了した時にバックアップファイルを復元しない</dd>
  <dt>-s, --skip-setup</dt><dd>初回起動時の設定ダイアログを表示しない</dd>
  <dt>-l, --logfile</dt>
  <dd>エラーなどのメッセージをファイル(キャッシュディレクトリの<code>log/msglog</code>)に出力する</dd>
  <dt>-g, --geometry WxH-X+Y</dt>
  <dd>幅(W)高さ(H)横位置(X)縦位置(Y)の指定。
  WxHは省略化(例: <code>-g 100x40-10+30</code>, <code>-g -20+100</code> )</dd>
  <dt>-V, --version</dt><dd>バージョン及びconfigureオプションを全て表示</dd>
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

- 起動するかどうか確認してサブプロセスを起動
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
廃止されたGTK2版同様のルック・アンド・フィールになるように実装しているが、
技術的な問題やテスト不足から完全な再現はできていない。

#### Wayland対応
JDim はWayland環境で起動するが動作は安定していない。
GTKのバックエンドにWaylandを使うかわりに互換レイヤーのXWaylandをインストールして使う。
環境変数 `GDK_BACKEND=x11` を設定してjdimを起動する。
```sh
# シェルからJDimを起動する場合
GDK_BACKEND=x11 ./src/jdim
```

WaylandやXWaylandではX11限定の機能を使うことができないため注意すること。

#### GTK2版から変更/追加された部分
* GTK+ 3.14以上の環境でタッチスクリーンによる操作に対応した。
  スレビューのタッチ操作については[操作方法について][]を参照。
* 書き込みビューの配色にGTKテーマを使う設定が追加された。
  1. メニューバーの`設定(C) > フォントと色(F) > 詳細設定(R)...`からフォントと色の詳細設定を開く
  2. `色の設定`タブにある`書き込みビューの配色設定に GTKテーマ を用いる(W)`をチェックして適用する
* GTK 3.16以上の環境で書き込みビューのダブルクリックによる単語の範囲選択、
  トリプルクリックによる行の範囲選択に対応した。

#### 既知の問題
* タブのドラッグ・アンド・ドロップの矢印ポップアップの背景が透過しない環境がある。
  (アルファチャンネルが利用できない環境)
* Wayland上で起動したときポップアップ内のアンカーからポップアップを出すとマウスポインターから離れた位置に表示される。
* Weston(Waylandコンポジタ)環境でXWaylandをバックエンドに指定して起動した場合、右クリックしながらポップアップ内に
  マウスポインターを動かすとポップアップ内容ではなくポップアップに隠れたスレビューに反応する。
* 32bit OSの古いMATE環境（バージョン[1.10][mate-1-10]から[1.16][mate-1-16]？）でJDim GTK3版を実行したとき
  スレビューの上に別のウインドウを重ねて移動させると残像でスレビュー内のみ描画が乱れる。
  スレビューをスクロール等させて再描画すると直る。([背景事情][mate-background])

[mate-1-10]: https://mate-desktop.org/blog/2015-06-11-mate-1-10-released/ "GTK3の実験的なサポート追加"
[mate-1-16]: https://mate-desktop.org/blog/2016-09-21-mate-1-16-released/ "GTK3に移行途中"
[mate-background]: https://github.com/JDimproved/JDim/commit/ffbce60ede#commitcomment-40911816 "別の不具合が再発する"


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

- デフォルトのキャッシュディレクトリが異なる (`~/snap/jdim/common/.cache/jdim/`)
- **デフォルトのキャッシュディレクトリはパッケージを削除すると消去される**
- JDのキャッシュディレクトリ (`~/.jd`) は使えない
- 隠しファイル（ドットファイル）のキャッシュディレクトリは使えない
- 外部コマンドの呼び出しは制限される
- 環境によってはGTKテーマ、アイコン、マウスカーソルがうまく表示されない場合がある<br>
  環境変数 (`GTK_THEME`) やGTKの設定ファイル (`$XDG_CONFIG_HOME/gtk-3.0/settings.ini`)
  を調整することで改善できるかもしれない


[pr-merged]: https://github.com/JDimproved/JDim/pulls?q=is%3Apr+is%3Amerged
[snapcraft]: https://snapcraft.io/jdim

[操作方法について]: {{ site.baseurl }}/operation/#threadview_touch "操作方法について \| JDim"
