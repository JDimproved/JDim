<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="動作環境の記入について">


<group header="概要">
  <sentence>
  書き込みビューのコンテキストメニューからJDの動作環境が記入できる。不具
  合報告などの際に活用すること。
  </sentence>

<asciiart caption="自動的に入力される物の例">
[バージョン] 2.0.3-090305
[ディストリ ] Example Linux
[パッケージ] バイナリ/ソース( &lt;配布元&gt; )
[ DE／WM ] GNOME
[　gtkmm 　] 2.14.3
[　glibmm 　] 2.18.1
[オプション ] '--with-alsa'
[ そ の 他 ]
</asciiart>

  <sentence>
  バージョン等は自動で入力されるが、他の項目については追加/変更すること。
  </sentence>
</group>


<group header="注意">
  <list type="circle">
    <item>「[パッケージ]」の項目に自動で表示される物はテンプレートなので明らかに不要と考えられる場合を除いて変更する事。</item>
    <item>「[ DE／WM ]」の項目は環境変数から判別出来た場合のみ入る。</item>
    <item>「[オプション ]」の項目はconfigureオプションの一部が入る。</item>
    <item>SVN版のバージョンはビルド時にリビジョン番号が取得できた場合にはそれになるが、そうでない場合は日時形式になる。</item>
    <item>makeはソースのルートディレクトリ(top_builddir)で実行しないとリビジョン番号が取得/更新されないので注意。</item>
  </list>
</group>


<group header="記載例">

<asciiart>
[バージョン] 2.0.3-090305
[ディストリ ] Ubuntu 8.10
[パッケージ] バイナリ( Ubuntu-ja )
[ DE／WM ] GNOME
[　gtkmm 　] 2.14.1
[　glibmm 　] 2.18.1
[オプション ]
[ そ の 他 ]
</asciiart>

<asciiart>
[バージョン] 2.0.3-090305
[ディストリ ] Fedora release 10 (Cambridge)
[パッケージ] ソース( Fedora SRPM )
[ DE／WM ] KDE + compiz-0.7.8
[　gtkmm 　] 2.14.3
[　glibmm 　] 2.18.1
[オプション ] '--with-sessionlib=gnomeui'
[ そ の 他 ]
</asciiart>

<asciiart>
[バージョン] SVN Rev.2701
[ディストリ ] Gentoo Base System release 1.12.11.1( x86_64 )
[パッケージ] ソース( JD project )
[ DE／WM ] fluxbox
[　gtkmm 　] 2.14.3
[　glibmm 　] 2.18.1
[オプション ] '--with-alsa'
'--eneble-gprof'
[ そ の 他 ] ebuild 使用
</asciiart>

</group>


</document>
