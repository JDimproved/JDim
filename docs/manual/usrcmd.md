<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="ユーザーコマンド、リンクフィルタについて">


<group header="登録">
  <sentence>
  スレビューや画像ビューで右クリックしたときに表示されるコンテキストメニューにユーザ
  コマンドを登録できる。
  </sentence>

  <sentence>
  登録するにはメニューの「設定」→「その他」→「ユーザコマンドの編集」か
  ら設定ダイアログを開き、右クリックしてコンテキストメニューから「新規コ
  マンド」を選択する。同様にディレクトリや区切り線も作成できる。
  </sentence>

  <sentence>
  「選択不可のユーザコマンドを非表示にする」をチェックすると選択出来ない
  コマンドは表示されなくなる。
  </sentence>
</group>


<group header="編集">
  <sentence>  
  一度設定したコマンドを編集する場合は、設定ダイアログで対象のコマンドを
  ダブルクリックする。また、行をドラッグして位置を変えたり階層構造にする
  ことが出来る。行を削除する場合はクリックしてからdeleteキーを押すかコン
  テキストメニューから削除する。
  </sentence>

  <sentence>
  なお、実行するコマンドでは下記のような置換文字列を指定出来る。また、コ
  マンドを手動で編集したい場合はキャッシュディレクトリにある&quot;usrcmd.xml&quot;
  というXMLファイルを編集する。
  </sentence>
</group>


<group header="置換文字一覧">
  <sentence>
  ユーザコマンドの具体的な使いかたは <text link="http://sourceforge.jp/projects/jd4linux/wiki/%E3%83%A6%E3%83%BC%E3%82%B6%E3%83%BC%E3%82%B3%E3%83%9E%E3%83%B3%E3%83%89%E8%A8%AD%E5%AE%9A%E9%9B%86">ユーザーコマンド設定集</text> (wiki) を参照すること。
  </sentence>

  <sentence>
   以下の説明はスレビューでユーザーコマンドを使用する場合であるので、画像ビューで使用する場合は $URL 等は画像が貼ってあったスレのアドレス、$LINK 等は画像のアドレス、$CACHEDIMG は画像キャッシュのパスに読み替える。
  </sentence>

  <descriptions>
    <item header="$VIEW">ネットワーク設定で指定されたwebブラウザで開く ( 例: $VIEW $LINK でリンクをブラウザで開く )</item>
    <item header="$DIALOG">$DIALOGの後の文字列をダイアログ表示</item>
    <item header="$ONLY">スレのURLが$ONLYの後の文字列(正規表現)を含む時だけメニューを表示(下記参照)</item>
    <item header="$URL">スレのURL ( 例: http://hibari.2ch.net/test/read.cgi/linux/1276299375/ )</item>
    <item header="$DATURL">サーバー上のdatファイルのURL ( 例: http://hibari.2ch.net/linux/dat/0123456789.dat )</item>
    <item header="$SERVER">http://などのプロトコルを含むスレのURLから取り出したサーバー名 ( 例: http://hibari.2ch.net )</item>
    <item header="$HOSTNAME">http://などのプロトコルを除くスレのURLから取り出したサーバー名 ( 例: hibari.2ch.net )</item>
    <item header="$HOST">$HOSTNAMEと同じ</item>
    <item header="$OLDHOSTNAME">$HOSTNAMEと同じだが、もしスレが板移転前に立てられたものなら移転前のサーバー名を使用する</item>
    <item header="$OLDHOST">$OLDHOSTNAMEと同じ</item>
    <item header="$BBSNAME">スレが属する板のID ( 例: http://hibari.2ch.net/linux なら linux )</item>
    <item header="$DATNAME">スレのID ( 例: http://hibari.2ch.net/test/read.cgi/linux/1276299375/ なら 1276299375 )</item>
    <item header="$LINK">マウスの下にリンクがあればそのURL</item>
    <item header="$SERVERL">http://などのプロトコルを含むリンクから取り出したサーバー名( 例: http://www.google.co.jp )</item>
    <item header="$HOSTNAMEL">http://などのプロトコルを除くリンクから取り出したサーバー名( 例: www.google.co.jp )</item>
    <item header="$HOSTL">$HOSTNAMELと同じ</item>
    <item header="$OLDHOSTNAMEL">$HOSTNAMELと同じだが、もしリンク先が2chのスレの場合は板移転前のサーバー名を使用する</item>
    <item header="$OLDHOSTL">$OLDHOSTNAMELと同じ</item>
    <item header="$BBSNAMEL">もしリンク先が2chのスレの場合は、そのスレが属する板のID ( 例: http://hibari.2ch.net/linux なら linux )</item>
    <item header="$DATNAMEL">もしリンク先が2chのスレの場合は、そのスレのID ( 例: http://hibari.2ch.net/test/read.cgi/linux/1276299375/ なら 1276299375 )</item>
    <item header="$CACHEDIMG">リンクが画像でかつキャッシュされている時は画像キャッシュの場所</item>
    <item header="$LOGPATH">キャッシュディレクトリの場所 ( 例: ~/.jd/ )</item>
    <item header="$LOCALDAT">キャッシュディレクトリにあるdatファイルの場所 ( 例: ~/.jd/hibari.2ch.net/linux/1276299375.dat )</item>
    <item header="$LOCALDATL">もしリンク先が2chのスレの場合は、キャッシュディレクトリにあるdatファイルの場所</item>
    <item header="$TITLE">スレのタイトル(UTF-8)</item>
    <item header="$NUMBER">スレビューの場合はマウスカーソルの下にあるレス番号、画像ビューの場合は参照元のレス番号</item>
    <item header="$BOARDNAME">スレが属する板の名前(UTF-8)</item>
    <item header="$TEXT">範囲選択した文字列(UTF-8)</item>
    <item header="$TEXTI">範囲選択した文字列(UTF-8)、選択がないときは入力ダイアログを表示する</item>
    <item header="$TEXTU">範囲選択した文字列(UTF-8)をURLエンコード</item>
    <item header="$TEXTIU">範囲選択した文字列(UTF-8)をURLエンコード、選択がないときは入力ダイアログを表示する</item>
    <item header="$TEXTX">範囲選択した文字列をEUCに変換してURLエンコード</item>
    <item header="$TEXTIX">範囲選択した文字列をEUCに変換してURLエンコード、選択がないときは入力ダイアログを表示する</item>
    <item header="$TEXTE">範囲選択した文字列をSJIS(MS932)に変換してURLエンコード</item>
    <item header="$TEXTIE">範囲選択した文字列をSJIS(MS932)に変換してURLエンコード、選択がないときは入力ダイアログを表示する</item>
    <item header="$INPUT">入力ダイアログを表示(UTF-8)</item>
    <item header="$INPUTU">入力ダイアログを表示してURLエンコード(UTF-8)</item>
    <item header="$INPUTX">入力ダイアログを表示しEUCに変換してURLエンコード</item>
    <item header="$INPUTE">入力ダイアログを表示しSJIS(MS932)に変換してURLエンコード</item>
  </descriptions>

  <subgroup header="$ONLYの例">
    <sentence>
    以下のように設定すると、スレのURLにlinuxを含むときのみメニューに表示
    されて「Linux」とダイアログ表示する。
    </sentence>

    <dialog>
      <item label="コマンド名" value="linuxのみ" />
      <item label="実行するコマンド" value="$ONLY linux $DIALOG Linux" />
    </dialog>

    <sentence>
    以下のコマンドはスレのURLにnewsを含むときのみメニューに表示されて
    「ニュース」とダイアログ表示する。
    </sentence>

    <dialog>
      <item label="コマンド名" value="newsのみ" />
      <item label="実行するコマンド" value="$ONLY news $DIALOG ニュース" />
    </dialog>
  </subgroup>
</group>


<group header="リンクフィルタ">
  <sentence>
  スレビューでリンクをクリックした時、アドレス毎にフィルタリングを行って
  動作を変えることが出来る。アドレスは正規表現として解釈され、フィルタの
  上の方からマッチングしていく。
  </sentence>

  <subgroup header="設定と編集">
    <sentence>
    フィルタの条件を設定するにはメニューの「設定」→「その他」→「リンク
    フィルタの編集」から設定ダイアログを開き、追加ボタンを押してアドレス
    と実行するコマンドを指定する。一度設定した条件を編集する場合は対象の
    条件をダブルクリックする。削除する場合は対象の条件をクリックしてから
    deleteキーを押すか削除ボタンを押す。
    </sentence>

    <sentence>
    順番を変えるには行を選択してから上端ボタンや上へボタンなどを押す。
    コマンドには上のユーザコマンドと同じ置換文字を使える。
    また、リンクフィルタのコマンドでは以下の置換文字を使用できる。
      <descriptions>
        <item header="\0">アドレスの正規表現にマッチした文字列</item>
        <item header="\1〜\9">アドレスの正規表現で「(...)」にマッチした部分文字列</item>
      </descriptions>
    </sentence>

    <sentence>
    なお、コマンドを手動で編集したい場合はキャッシュディレクトリにある
    「linkfilter.xml」というXMLファイルを編集する。
    </sentence>
  </subgroup>

  <subgroup header="使用例">
    <sentence>
    以下のように設定すると hoge.net のjpg画像はeogで開くが、その他のhoge.net
    上にある画像やファイルは外部ブラウザで開く。
    </sentence>

    <dialog>
      <item label="アドレス" value="http://hoge\.net/.*jpg$" />
      <item label="実行するコマンド" value="eog $LINK" />
    </dialog>

    <dialog>
      <item label="アドレス" value="http://hoge\.net/" />
      <item label="実行するコマンド" value="$VIEW $LINK" />
    </dialog>

    <sentence>
    また、以下ではパスにburakuraを含むリンクをクリックすると「ブラクラ」
    というダイアログを表示する。どの条件にもマッチしなかった場合は通常の
    動作になる。
    </sentence>

    <dialog>
      <item label="アドレス" value="burakura" />
      <item label="実行するコマンド" value="$DIALOG ブラクラ" />
    </dialog>
  </subgroup>
</group>


</document>
