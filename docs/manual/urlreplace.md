---
title: URL変換について
layout: default
---

&gt; [Top](../) &gt; [その他]({{ site.baseurl }}/info/) &gt; {{ page.title }}

## {{ page.title }}

- [URL変換](#replacement)
- [設定ファイル](#configfile)
- [設定例](#example)


<a name="replacement"></a>
### URL変換

書き込みされたURLと実際に表示する画像のURLが異なる場合に、
URLを正規表現で変換することで直接画像にアクセスできる。
また、リファラを送信したり、拡張子のないURLを強制的に画像と認識させることができる。


<a name="configfile"></a>
### 設定ファイル
URL変換機能を使う場合は、キャッシュディレクトリの「**urlreplace.conf**」ファイルに設定する。
設定ファイルを変更したあとは、一度JDimを再起動することで設定内容が有効になる。

youtubeのサムネイルをインライン画像として表示するための変換が、デフォルトで設定される。
なお、設定内容をリセットしたい場合は、「urlreplace.conf」ファイルを削除してからJDimを起動すると、
ファイルが自動で再作成される。

#### 設定ファイルの書式
```
正規表現<タブ>変換後URL<タブ>リファラURL<タブ>制御文字
```

制御ファイルに指定した、正規表現はファイルの先頭から順に評価される。
また、リファラURLおよび制御文字は、変換した後のURLが、正規表現に一致したときに有効になる。

#### 変換後URLおよびリファラURL
正規表現に一致した文字列をもとに、以下の置換文字を使用できる。

| 置換文字 | 内容 |
| --- | --- |
| `\0` または `$0` | 正規表現にマッチした文字列 |
| `\1`〜`\9` または `$1`〜`$9` | 正規表現で「(...)」にマッチした部分文字列 |

#### 制御文字

<dl>
  <dt>$IMAGE</dt>
  <dd>拡張子がない場合でも画像として扱う。 </dd>
  <dt>$BROWSER</dt>
  <dd>拡張子があっても画像として扱わない。 </dd>
  <dt>$GENUINE</dt>
  <dd>JDimは拡張子と実際の画像形式が不一致のときに、画像が偽装されていると判断して モザイクで表示する。
    これを指定すると、拡張子の偽装をチェックせず、モザイクで表示しない。<br>
    また、WebPやAVIFを表示できる環境ではwebサイトが元画像のかわりに送ることを容認する。<small>(v0.5.0+)</small>
    </dd>
  <dt>$BREAK</dt>
  <dd>正規表現に一致したら以降の判定を行わず、評価を終了する。</dd>
  <dt>$THUMBNAIL</dt>
  <dd>変換した後のURLが、動画投稿サイトなどのサムネイル画像として扱う。
    画像表示設定でインライン画像を表示しているときに有効となる。</dd>
</dl>


<a name="example"></a>
### 設定例
リファラURLを送信したり制御文字を有効にするには、変換後URLが正規表現に一致するように表現する。
変換後のURLを変換用の条件とリファラ用の条件を、2行に分けて書いてもよい。

<dl>
  <dt>1行でまとめて書く場合</dt>
  <dd>
<pre><code>http://www\.foobar\.com/(view/|img\.php\?id=)([0-9]+)	http://www.foobar.com/view/$2	$0	$IMAGE
</code></pre>
  </dd>
  <dt>2行に分けて書く場合</dt>
  <dd>
<pre><code>http://www\.foobar\.com/img\.php\?id=([0-9]+)	http://www.foobar.com/view/$1
http://www\.foobar\.com/view/([0-9]+)	$0	$0	$IMAGE
</code></pre>
  </dd>
  <dt>imgurの拡張子無しURLを画像リンク(jpg)にする</dt>
  <dd>
<pre><code>^https?://imgur\.com/([0-9A-Za-z]{7})$	https://i.imgur.com/$1.jpg		$IMAGE
</code></pre>
  </dd>
  <dt>imgurの画像URLは偽装チェックしない</dt>
  <dd>
<pre><code>^https?://i\.imgur\.com/([^#&=/]+)$	$0		$GENUINE
</code></pre>
  </dd>
</dl>
