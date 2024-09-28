---
title: 動作環境の記入について
layout: default
---

&gt; [Top](../) &gt; [その他]({{ site.baseurl }}/info/) &gt; {{ page.title }}

## {{ page.title }}

- [概要](#abstract)
- [注意](#note)
- [記載例](#example)


<a name="abstract"></a>
### 概要

書き込みビューのコンテキストメニューからJDimの動作環境が記入できる。不具合報告などの際に活用すること。

#### 自動的に入力される物の例
```
[バージョン] JDim 0.12.0-20241001(git:abcde12345)
[ディストリ ] Example Linux (x86_64)
[パッケージ] バイナリ/ソース( <配布元> )
[ DE／WM ] GNOME (Wayland)
[　gtkmm 　] 3.24.9
[　glibmm 　] 2.66.7
[　TLS lib　] GnuTLS 3.8.3
[オプション ] '--with-alsa'
[ そ の 他 ]
```

バージョン等は自動で入力されるが、他の項目については追加/変更すること。


<a name="note"></a>
### 注意

- 「`[バージョン]`」はgitのコミットハッシュが取得できた場合はコミット作成日と `(git:〜)` が追加される。
  このときローカルのソースコードに変更があるときはマーク(`:M`)が付く。
  ハッシュを取得できなかったときは [ヘッダーファイル (jdversion.h)][jdversion] の日付が追加される。
- 「`[パッケージ]`」の項目は、パッケージ作成者が設定したパッケージ情報が表示される。
  または、情報が未設定の場合はテンプレート（`バイナリ/ソース( <配布元> )`）が表示される。
- 「`[ DE／WM ]`」の項目は環境変数から判別出来た場合のみ入る。
  ディスプレイサーバーの種類は v0.13.0-alpha20240928 から追加。
- 「`[　TLS lib　]`」の項目はバージョン 0.2.0 から追加。
- 「`[オプション ]`」の項目はビルドオプションの一部が入る。( 無い場合は省略 )


<a name="example"></a>
### 記載例
引用: [5ch ブラウザ JD 21][thread] (5chスレ)

```
[バージョン] JDim 0.1.0-20190126(git:71f2266d0f)
[ディストリ ] Linux Mint 19.1 (x86_64)
[パッケージ] バイナリ/ソース( <配布元> )
[ DE／WM ] GNOME
[　gtkmm 　] 2.24.5
[　glibmm 　] 2.56.0
[ そ の 他 ]
```

```
[バージョン] JDim 0.1.0-20190217(git:e8cc28c993)
[ディストリ ] PCLinuxOS 2019 (x86_64)
[パッケージ] バイナリ/ソース( <配布元> )
[ DE／WM ] KDE
[　gtkmm 　] 2.24.5
[　glibmm 　] 2.56.1
[ そ の 他 ] LANG = en_US.UTF-8
```

```
[バージョン] JDim 0.1.0-20190302(git:b959c45b2a)
[ディストリ ] Manjaro Linux (x86_64)
[パッケージ] バイナリ/ソース( <配布元> )
[ DE／WM ] XFCE
[　gtkmm 　] 3.24.0
[　glibmm 　] 2.58.0
[オプション ] '--with-stdthread'
'--with-openssl''--with-gtkmm3'
[ そ の 他 ]
```


[jdversion]: https://github.com/JDimproved/JDim/tree/master/src/jdversion.h "JDim/jdversion.h at master"
[thread]: https://mao.5ch.net/test/read.cgi/linux/1540656394/
