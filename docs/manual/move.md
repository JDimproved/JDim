<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="板移転について">


<group header="移転処理">
  <sentence>
  「ファイル」メニューから板リストを再読込したときに移転した板があるとき
  は自動で移転処理する。なお移転処理をしたあとは一度JDを再起動することを
  推奨する。
  </sentence>

  <sentence>
  移転情報テーブルはキャッシュディレクトリ内の「move.info」ファイルに記
  録されるのでもしテーブルが壊れたときは手動で修正すること。
  </sentence>

<example caption="move.infoの形式">
旧サーバー 新サーバー /板ID
</example>

<example caption="実際の例">
http://pc2.2ch.net http://pc8.2ch.net /linux
</example>
</group>


</document>
