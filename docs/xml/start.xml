<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="起動について">


<group header="通常の起動">
  <sentence>使い方は以下のとおり。</sentence>
  <shell>$ jd [OPTION] [URL,FILE]</shell>

  <sentence>
  引数にURLを付けて起動する事も出来るので、他のアプリケーションから外部
  コマンドとしてURLを開く事などが出来る。(JDが扱う事の出来るURLでない場
  合は設定されているWebブラウザに渡される)
  </sentence>
  <shell>$ jd http://pc99.2ch.net/test/read.cgi/linux/1234567890/</shell>

  <sentence>
   ローカルにあるdatファイルを指定して、一時的にスレビュー表示させることも出来る。
  </sentence>
  <shell>$ jd ./12345.dat</shell>

  <sentence>
  環境変数 JD_CACHE でキャッシュディレクトリの位置を変更・指定することが可能。
  指定しなければ ~/.jd がキャッシュディレクトリになる。
  </sentence>

  <shell>$ JD_CACHE=~/.mycache jd</shell>

  <sentence>
  環境変数 JD_LOCK でロックファイルの位置を変更・指定することが可能。
  指定しなければ ~/.jd/JDLOCK がロックファイルになる。
  </sentence>

  <shell>$ JD_LOCK=~/mylock jd</shell>

  <descriptions caption="オプション">
    <item header="-h, --help">ヘルプを表示</item>
    <item header="-m, --multi">多重起動時のサブプロセスであっても終了しない</item>
    <item header="-n, --norestore">前回異常終了した時にバックアップファイルを復元しない</item>
    <item header="-s, --skip-setup">初回起動時の設定ダイアログを表示しない</item>
    <item header="-l, --logfile">エラーなどのメッセージをファイル(キャッシュディレクトリのlog/msglog)に出力する</item>
    <item header="-g, --geometry WxH-X+Y">幅(W)高さ(H)横位置(X)縦位置(Y)の指定。WxHは省略化(例: -g 100x40-10+30, -g -20+100 )</item>
    <item header="-V, --version">バージョン及びconfigureオプションを全て表示</item>
  </descriptions>
</group>


<group header="多重起動について">
  <sentence>JDはメインプロセス/サブプロセスという関係で動作する。</sentence>

  <list type="none">
    <item>メインプロセス: 指令を受け取る事が出来るプロセス</item>
    <item>サブプロセス　: 指令を出す事が出来るプロセス</item>
  </list>

  <sentence>
  通常は最初に起動した物がメインプロセスとなり、メインプロセスは1つだけ存
  在する事が出来る。メインプロセスが存在する状態で起動したプロセスはサブ
  プロセスとして扱われ、複数存在させる事も可能。なお、指令を受け取るのは
  メインプロセスのみなので、指令を出す側のサブプロセスでURLは開かれない。
  </sentence>

  <sentence>
  以下のコマンドを使い分ける事でサブプロセスの起動のしかたをコントロール
  出来る。
  </sentence>

  <list type="alpha">
    <item>
    起動するかどうか確認してサブプロセスを起動
    <shell>$ jd</shell>
    </item>
    <item>
    確認せずにサブプロセスを起動
    <shell>$ jd -m</shell>
    </item>
    <item>
    メインプロセスに&lt;URL&gt;を渡してサブプロセスを起動
    <shell>$ jd -m http://pc99.2ch.net/test/read.cgi/linux/1234567890/</shell>
    </item>
  </list>

  <sentence>
  注: サブプロセスを残したままメインプロセスを終了していた場合は次に起動
  したプロセスがメインプロセスとなる。
  </sentence>
</group>


</document>
