<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="make、実行方法について">


<group header="動作環境">
  <subgroup header="必須環境">
    <list type="circle">
      <item>gtkmm-2.4.8 以上</item>
      <item>zlib-1.2 以上</item>
      <item>gnutls-1.2 以上</item>
    </list>
  </subgroup>

  <subgroup header="推奨環境">
    <list type="circle">
      <item>Linux Kernel 2.6以上</item>
      <item>gtkmm-2.8以上 ( 2.6より低いとレイアウトが一部崩れる、2.8より低いとスレ一覧表示が遅い )</item>
      <item>UTF-8環境 ( EUC環境では LANG=&quot;ja_JP.UTF-8&quot; を指定する必要がある )</item>
    </list>
  </subgroup>
</group>


<group header="makeに必要なツール、ライブラリ">
  <subgroup header="必須">
    <list type="circle">
      <item>autoconf</item>
      <item>automake</item>
      <item>g++</item>
      <item>gnutls</item>
      <item>gtkmm</item>
      <item>libtool</item>
      <item>make</item>
      <item>zlib</item>
    </list>
  </subgroup>

  <subgroup header="オプション">
    <list type="circle">
      <item>alsa-lib (--with-alsa)</item>
      <item>libgnomeui (--with-sessionlib=gnomeui)</item>
      <item>openssl (--with-openssl)</item>
      <item>oniguruma (--with-oniguruma)</item>
      <item>libpcre (--with-pcre)</item>
    </list>
  </subgroup>

  <sentence>
  OSやディストリビューション別の解説は<text link="http://sourceforge.jp/projects/jd4linux/wiki/OS%2f%E3%83%87%E3%82%A3%E3%82%B9%E3%83%88%E3%83%AA%E3%83%93%E3%83%A5%E3%83%BC%E3%82%B7%E3%83%A7%E3%83%B3%E5%88%A5%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E6%96%B9%E6%B3%95">OS/ディストリビューション別インストール方法</text> (wiki) を参照。
  </sentence>
</group>


<group header="make 方法( rpmbuild の場合 )">
  <list type="decimal">
    <item>rpmbuild -tb 〜.tgz でrpmファイルが出来るのであとは rpm -Uvh 〜.rpm</item>
    <item>ライブラリが足りないといわれたら yum install 〜-devel</item>
    <item>起動はメニューから起動するか、端末で jd と打ち込んでエンターを押す。</item>
  </list>
</group>


<group header="make 方法( configure + make の場合 )">
  <list type="decimal">
    <item>autoreconf -i ( 又は ./autogen.sh )</item>
    <item>./configure</item>
    <item>make</item>
    <item>(お好みで) strip src/jd</item>
  </list>
</group>


<group header="configureオプション">
  <descriptions>
    <item header="--with-sessionlib=[xsmp|gnomeui|no]">
    GNOMEUIを使ってセッション管理をするには「gnomeui」を、セッション管理
    を無効にするには「no」を選択。デフォルトでは XSMPを使用する。
    </item>
    <item header="--with-pangolayout">
    描画にPangoLayoutを使う。デフォルトでは PangoGlyphString を使用する。
    </item>
    <item header="--with-migemo">
    migemoによる検索が有効になる。migemoがUTF-8の辞書でインストールされている必要がある。
    </item>
    <item header="--with-[native|core2duo|athlon64|atom|ppc7400|ppc7450]">
    CPUに合わせた最適化
    </item>
    <item header="--with-openssl">
    GNU TLSではなく OpenSSLを使用する。ライセンス上バイナリ配布が出来なくなることに注意すること。
    </item>
    <item header="--with-alsa">
    ALSAによる効果音再生機能を有効にする。
    詳しくは<text link="sound.xml">効果音の再生</text>の項を参照すること。
    </item>
    <item header="--with-xdgopen">
    デフォルトブラウザとしてxdg-openを使用する。
    </item>
    <item header="--enable-gprof">
    gprofによるプロファイリングを行う。
    コンパイルオプションに -pg が付き、JDを実行すると gmon.out が出来るので
    gprof  ./jd  gmon.out で解析できる。CPUの最適化は効かなくなるので注意する。
    </item>
    <item header="--with-oniguruma">
    正規表現ライブラリとしてPOSIX regex の代わりに鬼車を使用する。鬼車はBSDライセンスなのでJDをバイナリ配布する場合には注意すること(ライセンスはGPLになる)。
    </item>
    <item header="--with-pcre">
    正規表現ライブラリとしてPOSIX regex の代わりにPCREを使用する。PCREはBSDライセンスなのでJDをバイナリ配布する場合には注意すること(ライセンスはGPLになる)。
    UTF-8が有効な ( --enable-utf オプションを用いて make する ) PCRE 6.5 以降が必要となる。
    Perl互換の正規表現なので、従来の POSIX 拡張の正規表現から設定変更が必要になる場合がある。
    </item>
    <item header="--with-gthread">
    pthreadの代わりにgthreadを使用する。
    </item>
  </descriptions>
</group>


<group header="メモ">
  <sentence>
  最近のディストリビューションの場合は autogen.sh よりも autoreconf -i の方を推奨。
  </sentence>

  <sentence>
  実行するには直接 src/jd を起動するか手動で /usr/bin あたりに src/jd を cp する。
  </sentence>

  <sentence>
  以上の操作でmakeが通らなかったり動作が変な時は configure のオプションを変更する。
  </sentence>
</group>


</document>
