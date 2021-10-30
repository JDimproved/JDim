---
title: テーマについて
layout: default
---

&gt; [Top](../) &gt; [その他]({{ site.baseurl }}/info/) &gt; {{ page.title }}

## {{ page.title }}

- [スタイルシート設定 (jd.css)](#stylesheet)
- [レス構造設定 (Res.html)](#reshtml)
- [フォント設定](#fonts)
- [アイコン設定](#icons)
- [使用例](#example)


<a name="stylesheet"></a>
### スタイルシート設定 (jd.css)
JDimはスタイルシートによるスレビュー表示のカスタマイズをサポートしている。
スタイルシートは[キャッシュディレクトリ][cachepriority]内にテーマフォルダ(`theme/`)を作成し
その中に「**jd.css**」という名前のファイルを作って設定する。
スタイルシートを変更したときはJDimを再起動する。

#### 対応しているプロバティ

| プロパティ名 | 説明 | 備考 |
| --- | --- | --- |
| color | 文字色 | リンクなどJDimの設定メニューから設定する物を除く |
| background-color | 背景色 | |
| border-color | 枠線色 | border-left-color など個別指定も可能 |
| border-style | 枠線形 | solid のみに対応 |
| border-width | 枠線幅 | border-left-width など個別指定も可能 |
| margin | 外余白 | margin-left など個別指定も可能 |
| padding | 内余白 | padding-left など個別指定も可能  |
| text-align | 位置 | left, center, right のみに対応 |

**注意**: スタイルシートはfont系のプロパティに対応していない。
[フォント設定](#fonts)はメニューバーからダイアログを開いておこなう。

#### 色の指定方法
`black`や`white`などの定義済み色名か「`#RRGGBB`」形式で指定する。定義済み色名の種類は次の通り。

<style>.box { display: block; text-align: center; width: 10em; }</style>

- CSS Level 1
  - <span class="box" style="color: white; background: red;">red</span>
  - <span class="box" style="color: white; background: fuchsia;">fuchsia</span>
  - <span class="box" style="color: white; background: purple;">purple</span>
  - <span class="box" style="color: white; background: maroon;">maroon</span>
  - <span class="box" style="color: black; background: yellow;">yellow</span>
  - <span class="box" style="color: black; background: lime;">lime</span>
  - <span class="box" style="color: white; background: green;">green</span>
  - <span class="box" style="color: white; background: olive;">olive</span>
  - <span class="box" style="color: white; background: blue;">blue</span>
  - <span class="box" style="color: black; background: aqua;">aqua</span>
  - <span class="box" style="color: white; background: teal;">teal</span>
  - <span class="box" style="color: white; background: navy;">navy</span>
  - <span class="box" style="color: black; background: white;">white</span>
  - <span class="box" style="color: black; background: silver;">silver</span>
  - <span class="box" style="color: white; background: gray;">gray</span>
  - <span class="box" style="color: white; background: black;">black</span>
- CSS Level 2 <small>(v0.6.0+からサポート)</small>
  - <span class="box" style="color: black; background: orange;">orange</span>
- CSS Colors Level 3 <small>(v0.6.0+からサポート)</small>
  - X11色, 数が多いため下記リンク参照
- CSS Colors Level 4 <small>(v0.6.0+からサポート)</small>
  - <span class="box" style="color: white; background: rebeccapurple;">rebeccapurple</span>

外部リンク: [&lt;color&gt; - CSS: カスケーディングスタイルシート &#x7c; MDN][color_value]


#### 単位
`px`, `em` のみに対応。単位を省略すると `px` になる。

<a name="resstructure"></a>
#### レスの構造
スレ内のひとつひとつのレスは以下のような構造となっている。
`NUMBER`や`NAME`などの要素については「[レス構造設定](#reshtml)」の項で説明する。

```html
<div class="res">
<div class="title"><NUMBER/> <NAMELINK/>：<NAME/> <MAIL/>： <DATE/> <ID/></div>
<div class="mes"><MESSAGE/><IMAGE/></div>
</div>
```

#### 定義済みのセレクタ
**注意**: jd.css でクラス指定する際は `div.res{}` と書かずに `.res{}`
と要素名(div)を書かずにクラス名だけを指定すること。

<dl>
  <dt>body</dt>
  <dd>スレビューのbody要素</dd>
  <dt>.res</dt>
  <dd>ひとつのレスの要素</dd>
  <dt>.title</dt>
  <dd>ひとつのレスのヘッダ行</dd>
  <dt>.mes</dt>
  <dd>ひとつのレスの本文</dd>
  <dt>.separator</dt>
  <dd>ここまで読んだ。<code>&lt;div class=&quot;separator&quot;&gt;ここまで読んだ&lt;/div&gt;</code>
    という構造になっている。</dd>
  <dt>.comment</dt>
  <dd>コメントブロック。<code>&lt;div class=&quot;comment&quot;&gt;任意のコメント&lt;/div&gt;</code>
    という構造になっている。</dd>
  <dt>imgpopup</dt>
  <dd>画像ポップアップのbody要素。color, background-color, border-color, border-width, margin
    プロバティのみ(単位はpxのみ)に対応。</dd>
</dl>


---

<a name="reshtml"></a>
### レス構造設定 (Res.html)
スレ内のひとつひとつのレスは「[レスの構造](#resstructure)」の項で示した構造となっている。
レスの構造は[キャッシュディレクトリ][cachepriority]内にテーマフォルダ(`theme/`)を作成し
その中に「**Res.html**」という名前のファイルを作って指定することが出来る。
レスの構造を変更したときはJDimを再起動する。

#### 使用可能な要素
`<div>` のみ

#### 置換可能な定義済み要素
Res.htmlでは次のように定義済み要素を指定して文字列の置換ができる。

| 定義済み要素 | 説明 |
| --- | --- |
| &lt;NUMBER/&gt; | レス番号 |
| &lt;NAMELINK/&gt; | 名前メニュー表示のリンク。「名前」の文字列に置換される。 |
| &lt;NAME/&gt; | 名前 |
| &lt;MAIL/&gt; | メール |
| &lt;DATE/&gt; | 日付 |
| &lt;ID/&gt; | ID |
| &lt;MESSAGE/&gt; | 本文。「br=&quot;no&quot;」属性を指定すると本文の改行をしない。 |
| &lt;IMAGE/&gt; | インライン画像 |

#### 注意点

- `<div class="res">〜</div>`はRes.htmlで指定する必要が無い。
  つまり Res.htmlに&quot;hoge&quot;とだけ指定した場合、レスの構造は以下のようになる。
  ```html
  <div class="res">
  hoge
  </div>
  ```
- div の中に div を配置することは出来ない( 上の `<div class="res">` は例外 )。
- クラス名は定義済みの物だけではなくて自由に指定できる。


---

<a name="fonts"></a>
### フォント設定
フォント設定はメニューバーの`設定(C)`＞`フォントと色(F)`から設定ダイアログを開いておこなう。

| 項目               | 適用範囲
| ---                | ---
| スレビュー         | スレビューのレス本文
| メール欄 &#x203B;1 | スレビューやポップアップのレス番号、名前、メール、日付、ID
| ポップアップ       | ポップアップウインドウのレス本文
| アスキーアート     | スレビューやポップアップのアスキーアートと判定されたレス本文
| 板一覧／お気に入り | 板一覧／お気に入りなどサイドバーの項目
| スレ一覧           | スレ一覧の列名と項目
| 書き込みビュー     | 書き込みビューの名前欄、メール欄、レス欄

- &#x203B;1 -- バージョン0.3.0-20200301から追加。それより前は「スレビュー」の適用範囲に含まれる。


---

<a name="icons"></a>
### アイコン設定
[キャッシュディレクトリ][cachepriority]内にアイコンテーマのフォルダ(`theme/icons/`)を作成し
その中に対応するアイコン画像を置くとJDimのアイコンが置き換わる。
アイコン画像を変更したときはJDimを再起動する。

- 画像形式は一般的な物なら大体使用可能 ( png, jpg, svg, gif, bmp, その他)
- 画像サイズは任意のサイズで良い( 16x16 がデフォルトサイズ)
- ファイル名は「アイコン名.拡張子 」(例) reload.jpg、 quit.png
- ファイル名の一覧はgitリポジトリにある [ヘッダファイル(iconfiles.h)][iconfiles] を参照すること


---

<a name="example"></a>
### 使用例
[![スキンサンプル][skin_sample_thumb]][skin_sample]
クリックで拡大

[キャッシュディレクトリ][cachepriority]内にテーマフォルダ(`theme/`)を作成し
その中にRes.html(下)と[jd.css][jdcss]をコピーしてJDimを起動する。

```html
<div class="title"><NUMBER/> <NAMELINK/>：<NAME/> <MAIL/></div>
<div class="mes"><MESSAGE/><IMAGE/></div>
<div class="id"><ID/></div>
<div class="date"><DATE/></div>
```


[color_value]: https://developer.mozilla.org/ja/docs/Web/CSS/color_value#color_keywords
[iconfiles]: https://github.com/JDimproved/JDim/blob/master/src/icons/iconfiles.h "JDim/iconfiles.h at master"

<!-- 相対URLで GitHub Pages と GitHub.com 両方に対応する -->
[skin_sample_thumb]: ../assets/skin_sample_thumb.png
[skin_sample]: ../assets/skin_sample.png "サンプルのスクリーンショット"
[jdcss]: ../assets/jd.css "サンプルのcss"
[cachepriority]:  ../start/#cachepriority "キャッシュディレクトリの優先順位"
