---
title: ユーザーコマンド、リンクフィルタについて
layout: default
---

&gt; [Top](../) &gt; {{ page.title }}

## {{ page.title }}

- [登録](#register)
- [編集](#edit)
- [置換文字一覧](#replacement)
- [リンクフィルタ](#filter)


<a name="register"></a>
### 登録
スレビューや画像ビューで右クリックしたときに表示されるコンテキストメニューにユーザコマンドを登録できる。

登録するにはメニューの「`設定`」→「`その他`」→「`ユーザコマンドの編集`」から設定ダイアログを開き、
右クリックしてコンテキストメニューから「`新規コマンド`」を選択する。同様にディレクトリや区切り線も作成できる。

「`選択不可のユーザコマンドを非表示にする`」をチェックすると選択出来ないコマンドは表示されなくなる。


<a name="edit"></a>
### 編集
一度設定したコマンドを編集する場合は、設定ダイアログで対象のコマンドをダブルクリックする。
また、行をドラッグして位置を変えたり階層構造にすることが出来る。
行を削除する場合はクリックしてからDeleteキーを押すかコンテキストメニューから削除する。

なお、実行するコマンドでは下記のような置換文字列を指定出来る。
また、コマンドを手動で編集したい場合はキャッシュディレクトリにある
&quot;usrcmd.xml&quot;というXMLファイルを編集する。


<a name="replacement"></a>
### 置換文字一覧
ユーザコマンドの具体的な使いかたは [ユーザーコマンド設定集][wiki-usrcmd] (JD wiki) を参照すること。

以下の説明はスレビューでユーザーコマンドを使用する場合であるので、画像ビューで使用する場合は `$URL`
等は画像が貼ってあったスレのアドレス、`$LINK` 等は画像のアドレス、`$CACHEDIMG` は画像キャッシュのパスに読み替える。

<dl>
  <dt>$VIEW</dt>
  <dd>ネットワーク設定で指定されたwebブラウザで開く ( 例: <code>$VIEW $LINK</code> でリンクをブラウザで開く )</dd>
  <dt>$DIALOG</dt>
  <dd><code>$DIALOG</code>の後の文字列をダイアログ表示</dd>

  <dt>$ONLY</dt>
  <dd>スレのURLが<code>$ONLY</code>の後の文字列(正規表現)を含む時だけメニューを表示
    (<a href="#only">下記参照</a>)</dd>

  <dt>$URL</dt>
  <dd>スレのURL ( 例: <code>http://hibari.2ch.net/test/read.cgi/linux/1276299375/</code> )</dd>
  <dt>$DATURL</dt>
  <dd>サーバー上のdatファイルのURL ( 例: <code>http://hibari.2ch.net/linux/dat/0123456789.dat</code> )</dd>
  <dt>$SERVER</dt>
  <dd>http://などのプロトコルを含むスレのURLから取り出したサーバー名 ( 例: <code>http://hibari.2ch.net</code> )</dd>
  <dt>$HOSTNAME</dt>
  <dd>http://などのプロトコルを除くスレのURLから取り出したサーバー名 ( 例: <code>hibari.2ch.net</code> )</dd>
  <dt>$HOST</dt>
  <dd><code>$HOSTNAME</code>と同じ</dd>
  <dt>$OLDHOSTNAME</dt>
  <dd><code>$HOSTNAME</code>と同じだが、もしスレが板移転前に立てられたものなら移転前のサーバー名を使用する</dd>
  <dt>$OLDHOST</dt>
  <dd><code>$OLDHOSTNAME</code>と同じ</dd>

  <dt>$BBSNAME</dt>
  <dd>スレが属する板のID ( 例: <code>http://hibari.2ch.net/linux</code> なら <code>linux</code> )</dd>
  <dt>$DATNAME</dt>
  <dd>スレのID ( 例: <code>http://hibari.2ch.net/test/read.cgi/linux/1276299375/</code> なら
    <code>1276299375</code> )</dd>
  <dt>$LINK</dt>
  <dd>マウスの下にリンクがあればそのURL</dd>
  <dt>$SERVERL</dt>
  <dd>http://などのプロトコルを含むリンクから取り出したサーバー名( 例: <code>http://www.google.co.jp</code> )</dd>
  <dt>$HOSTNAMEL</dt>
  <dd>http://などのプロトコルを除くリンクから取り出したサーバー名( 例: <code>www.google.co.jp</code> )</dd>
  <dt>$HOSTL</dt>
  <dd><code>$HOSTNAMEL</code>と同じ</dd>
  <dt>$OLDHOSTNAMEL</dt>
  <dd><code>$HOSTNAMEL</code>と同じだが、もしリンク先が2chのスレの場合は板移転前のサーバー名を使用する</dd>
  <dt>$OLDHOSTL</dt>
  <dd><code>$OLDHOSTNAMEL</code>と同じ</dd>
  <dt>$BBSNAMEL</dt>
  <dd>もしリンク先が2chのスレの場合は、そのスレが属する板のID ( 例: <code>http://hibari.2ch.net/linux</code> なら
    <code>linux</code> )</dd>
  <dt>$DATNAMEL</dt>
  <dd>もしリンク先が2chのスレの場合は、そのスレのID ( 例:
    <code>http://hibari.2ch.net/test/read.cgi/linux/1276299375/</code> なら <code>1276299375</code> )</dd>

  <dt>$CACHEDIMG</dt>
  <dd>リンクが画像でかつキャッシュされている時は画像キャッシュの場所</dd>
  <dt>$LOGPATH</dt>
  <dd>キャッシュディレクトリの場所 ( 例: <code>~/.jd/</code> )</dd>
  <dt>$LOCALDAT</dt>
  <dd>キャッシュディレクトリにあるdatファイルの場所 ( 例:
    <code>~/.jd/hibari.2ch.net/linux/1276299375.dat</code> )</dd>
  <dt>$LOCALDATL</dt>
  <dd>もしリンク先が2chのスレの場合は、キャッシュディレクトリにあるdatファイルの場所</dd>

  <dt>$TITLE</dt>
  <dd>スレのタイトル(UTF-8)</dd>
  <dt>$NUMBER</dt>
  <dd>スレビューの場合はマウスカーソルの下にあるレス番号、画像ビューの場合は参照元のレス番号</dd>
  <dt>$BOARDNAME</dt>
  <dd>スレが属する板の名前(UTF-8)</dd>

  <dt>$TEXT</dt>
  <dd>範囲選択した文字列(UTF-8)</dd>
  <dt>$TEXTI</dt>
  <dd>範囲選択した文字列(UTF-8)、選択がないときは入力ダイアログを表示する</dd>
  <dt>$TEXTU</dt>
  <dd>範囲選択した文字列(UTF-8)をURLエンコード</dd>
  <dt>$TEXTIU</dt>
  <dd>範囲選択した文字列(UTF-8)をURLエンコード、選択がないときは入力ダイアログを表示する</dd>
  <dt>$TEXTX</dt>
  <dd>範囲選択した文字列をEUCに変換してURLエンコード</dd>
  <dt>$TEXTIX</dt>
  <dd>範囲選択した文字列をEUCに変換してURLエンコード、選択がないときは入力ダイアログを表示する</dd>
  <dt>$TEXTE</dt>
  <dd>範囲選択した文字列をSJIS(MS932)に変換してURLエンコード</dd>
  <dt>$TEXTIE</dt>
  <dd>範囲選択した文字列をSJIS(MS932)に変換してURLエンコード、選択がないときは入力ダイアログを表示する</dd>

  <dt>$INPUT</dt>
  <dd>入力ダイアログを表示(UTF-8)</dd>
  <dt>$INPUTU</dt>
  <dd>入力ダイアログを表示してURLエンコード(UTF-8)</dd>
  <dt>$INPUTX</dt>
  <dd>入力ダイアログを表示しEUCに変換してURLエンコード</dd>
  <dt>$INPUTE</dt>
  <dd>入力ダイアログを表示しSJIS(MS932)に変換してURLエンコード</dd>
</dl>

<a name="only"></a>
#### $ONLYの例
以下のように設定すると、スレのURLにlinuxを含むときのみメニューに表示されて「Linux」とダイアログ表示する。

<dl>
  <dt>コマンド名</dt>
  <dd><pre><code>linuxのみ</code></pre></dd>
  <dt>実行するコマンド</dt>
  <dd><pre><code>$ONLY linux $DIALOG Linux</code></pre></dd>
</dl>

以下のコマンドはスレのURLにnewsを含むときのみメニューに表示されて「ニュース」とダイアログ表示する。

<dl>
  <dt>コマンド名</dt>
  <dd><pre><code>newsのみ</code></pre></dd>
  <dt>実行するコマンド</dt>
  <dd><pre><code>$ONLY news $DIALOG ニュース</code></pre></dd>
</dl>


<a name="filter"></a>
### リンクフィルタ
スレビューでリンクをクリックした時、アドレス毎にフィルタリングを行って動作を変えることが出来る。
アドレスは正規表現として解釈され、フィルタの上の方からマッチングしていく。

#### 設定と編集
フィルタの条件を設定するにはメニューの「`設定`」→「`その他`」→「`リンクフィルタの編集`」から設定ダイアログを開き、
追加ボタンを押してアドレスと実行するコマンドを指定する。
一度設定した条件を編集する場合は対象の条件をダブルクリックする。
削除する場合は対象の条件をクリックしてからDeleteキーを押すか削除ボタンを押す。

順番を変えるには行を選択してから上端ボタンや上へボタンなどを押す。
コマンドには上のユーザコマンドと同じ置換文字を使える。
また、リンクフィルタのコマンドでは以下の置換文字を使用できる。

| 置換文字 | 内容 |
| --- | --- |
| `\0` | アドレスの正規表現にマッチした文字列 |
| `\1`〜`\9` | アドレスの正規表現で「(...)」にマッチした部分文字列 |

なお、コマンドを手動で編集したい場合はキャッシュディレクトリにある「linkfilter.xml」というXMLファイルを編集する。

#### 使用例
以下のように設定すると hoge.net のjpg画像はeogで開くが、
その他の hoge.net 上にある画像やファイルは外部ブラウザで開く。

<dl>
  <dt>アドレス</dt>
  <dd><pre><code>http://hoge\.net/.&ast;jpg$</code></pre></dd>
  <dt>実行するコマンド</dt>
  <dd><pre><code>eog $LINK</code></pre></dd>
</dl>

<dl>
  <dt>アドレス</dt>
  <dd><pre><code>http://hoge\.net/</code></pre></dd>
  <dt>実行するコマンド</dt>
  <dd><pre><code>$VIEW $LINK</code></pre></dd>
</dl>

また、以下の例ではパスにburakuraを含むリンクをクリックすると「ブラクラ」というダイアログを表示する。
どの条件にもマッチしなかった場合は通常の動作になる。

<dl>
  <dt>アドレス</dt>
  <dd><pre><code>burakura</code></pre></dd>
  <dt>実行するコマンド</dt>
  <dd><pre><code>$DIALOG ブラクラ</code></pre></dd>
</dl>


[wiki-usrcmd]: https://ja.osdn.net/projects/jd4linux/wiki/%E3%83%A6%E3%83%BC%E3%82%B6%E3%83%BC%E3%82%B3%E3%83%9E%E3%83%B3%E3%83%89%E8%A8%AD%E5%AE%9A%E9%9B%86
