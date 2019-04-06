<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="URL変換について">


<group header="URL変換">
  <sentence>
    書き込みされたURLと実際に表示する画像のURLが異なる場合に、URLを正規表現で変換することで
    直接画像にアクセスできる。
    また、リファラを送信したり、拡張子のないURLを強制的に画像と認識させることができる。
  </sentence>
</group>


<group header="設定ファイル">
  <sentence>
    URL変換機能を使う場合は、キャッシュディレクトリの「urlreplace.conf」ファイルに設定する。
    設定ファイルを変更したあとは、一度JDを再起動することで設定内容が有効になる。
  </sentence>

  <sentence>
    youtubeのサムネイルをインライン画像として表示するための変換が、デフォルトで設定される。
    なお、設定内容をリセットしたい場合は、「urlreplace.conf」ファイルを削除してからJDを起動すると、
    ファイルが自動で再作成される。
  </sentence>

<example caption="設定ファイルの書式">
正規表現&lt;タブ&gt;変換後URL&lt;タブ&gt;リファラURL&lt;タブ&gt;制御文字
</example>

  <sentence>
    制御ファイルに指定した、正規表現はファイルの先頭から順に評価される。
    また、リファラURLおよび制御文字は、変換した後のURLが、正規表現に一致したときに有効になる。
  </sentence>

<subgroup header="変換後URLおよびリファラURL">
  <sentence>
    正規表現に一致した文字列をもとに、以下の置換文字を使用できる。
  </sentence>
  <descriptions>
    <item header="\0 または $0">正規表現にマッチした文字列</item>
    <item header="\1〜\9 または $1〜$9">正規表現で「(...)」にマッチした部分文字列</item>
  </descriptions>
</subgroup>

<subgroup header="制御文字">
  <descriptions>
    <item header="$IMAGE">
      拡張子がない場合でも画像として扱う。
    </item>
    <item header="$BROWSER">
      拡張子があっても画像として扱わない。
    </item>
    <item header="$GENUINE">
      JDは拡張子と実際の画像形式が不一致のときに、画像が偽装されていると判断して
      モザイクで表示する。これを指定すると、拡張子の偽装をチェックせず、モザイクで表示しない。
    </item>
    <item header="$BREAK">
      正規表現に一致したら以降の判定を行わず、評価を終了する。
    </item>
    <item header="$THUMBNAIL">
      変換した後のURLが、動画投稿サイトなどのサムネイル画像として扱う。
      画像表示設定でインライン画像を表示しているときに有効となる。
    </item>
  </descriptions>
</subgroup>

</group>


<group header="設定例">
  <sentence>
    リファラURLを送信したり制御文字を有効にするには、変換後URLが正規表現に一致するように表現する。
    変換後のURLを変換用の条件とリファラ用の条件を、2行に分けて書いてもよい。
  </sentence>

<example caption="1行でまとめて書く場合">
http://www\.foobar\.com/(view/|img\.php\?id=)([0-9]+)	http://www.foobar.com/view/$2	$0	$IMAGE
</example>

<example caption="2行に分けて書く場合">
http://www\.foobar\.com/img\.php\?id=([0-9]+)	http://www.foobar.com/view/$1
http://www\.foobar\.com/view/([0-9]+)	$0	$0	$IMAGE
</example>

</group>

</document>
