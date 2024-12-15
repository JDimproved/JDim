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
- [テーマ設定](#theme)


<a name="stylesheet"></a>
### スタイルシート設定 (jd.css)
JDimは、スタイルシートを使用してスレビュー表示をカスタマイズできます。
スタイルシートを設定するには、[キャッシュディレクトリ][cachepriority]内にテーマフォルダ(`theme/`)を作成し、
その中に「**jd.css**」という名前のファイルを作成します。
スタイルシートを変更したときは、JDimを再起動してください。

#### 対応しているプロパティ

| プロパティ名 | 説明 | 備考 |
| --- | --- | --- |
| color | 文字色 | リンクなどJDimの設定メニューから設定するものを除きます。 |
| background-color | 背景色 | |
| border-color | 枠線色 | border-left-color など個別指定も可能です。 |
| border-style | 枠線形 | solid のみに対応しています。 |
| border-width | 枠線幅 | border-left-width など個別指定も可能です。 |
| margin | 外余白 | margin-left など個別指定も可能です。 |
| padding | 内余白 | padding-left など個別指定も可能です。  |
| text-align | 位置 | left, center, right のみに対応しています。 |

- **注意:** スタイルシートでは、font系のプロパティは使用できません。
  フォントの設定は、[フォント設定](#fonts)ダイアログからおこなってください。

#### 色の指定方法
`black`や`white`などの定義済み色名か「`#RRGGBB`」形式で指定します。定義済み色名の種類は次のとおりです。

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
  - X11色, 数が多いため下記リンクを参照してください。
- CSS Colors Level 4 <small>(v0.6.0+からサポート)</small>
  - <span class="box" style="color: white; background: rebeccapurple;">rebeccapurple</span>

外部リンク: [&lt;color&gt; - CSS: カスケーディングスタイルシート &#x7c; MDN][color_value]


#### 単位
`px`, `em` のみに対応しています。単位を省略すると `px` になります。

<a name="resstructure"></a>
#### レスの構造
スレ内の各レスは、以下のような構造となっています。
`NUMBER`や`NAME`などの要素については「[レス構造設定](#reshtml)」の項で説明します。

```html
<div class="res">
<div class="title"><NUMBER/> <NAMELINK/>：<NAME/> <MAIL/>： <DATE/> <ID/></div>
<div class="mes"><MESSAGE/><IMAGE/></div>
</div>
```

#### 定義済みのセレクタ

- **注意**: jd.css でクラス指定する際は `div.res{}` と書かずに `.res{}`
  と要素名(div)を書かずにクラス名だけを指定してください。

<dl>
  <dt>body</dt>
  <dd>スレビューのbody要素です。</dd>
  <dt>.res</dt>
  <dd>各レスの要素です。</dd>
  <dt>.title</dt>
  <dd>各レスのヘッダ行です。</dd>
  <dt>.mes</dt>
  <dd>各レスの本文です。</dd>
  <dt>.separator</dt>
  <dd>「ここまで読んだ」を示す要素です。<code>&lt;div class=&quot;separator&quot;&gt;ここまで読んだ&lt;/div&gt;</code>
    という構造になっています。</dd>
  <dt>.comment</dt>
  <dd>コメントブロックを示す要素です。<code>&lt;div class=&quot;comment&quot;&gt;任意のコメント&lt;/div&gt;</code>
    という構造になっています。</dd>
  <dt>imgpopup</dt>
  <dd>画像ポップアップのbody要素です。color, background-color, border-color, border-width, margin
    プロパティのみ(単位はpxのみ)に対応しています。</dd>
</dl>


---

<a name="reshtml"></a>
### レス構造設定 (Res.html)
スレ内の各レスは、「[レスの構造](#resstructure)」の項で示した構造となっています。
レスの構造を変更するには、[キャッシュディレクトリ][cachepriority]内にテーマフォルダ(`theme/`)を作成し
その中に「**Res.html**」という名前のファイルを作成します。
レスの構造を変更した場合は、JDimを再起動してください。

#### 使用可能な要素
`<div>` のみ使用可能です。

#### 置換可能な定義済み要素
Res.htmlでは次のように定義済み要素を指定して文字列の置換ができます。

| 定義済み要素 | 説明 |
| --- | --- |
| &lt;NUMBER/&gt; | レス番号 |
| &lt;NAMELINK/&gt; | 名前メニュー表示のリンクです。「名前」の文字列に置換されます。 |
| &lt;NAME/&gt; | 名前 |
| &lt;MAIL/&gt; | メール |
| &lt;DATE/&gt; | 日付 |
| &lt;ID/&gt; | ID |
| &lt;MESSAGE/&gt; | 本文です。「br=&quot;no&quot;」属性を指定すると本文は改行しません。 |
| &lt;IMAGE/&gt; | インライン画像 |

#### 注意点

- `<div class="res">〜</div>`はRes.htmlで指定する必要はありません。
  つまり Res.htmlに&quot;hoge&quot;とだけ指定した場合、レスの構造は以下のようになります。
  ```html
  <div class="res">
  hoge
  </div>
  ```
- div の中に div を配置することはできません( 上の `<div class="res">` は例外です )。
- クラス名は定義済みのものだけではなく、自由に指定できます。


---

<a name="fonts"></a>
### フォント設定
フォント設定は、メニューバーの`設定(C)`＞`フォントと色(F)`から設定ダイアログを開いて行ってください。

| 項目               | 適用範囲
| ---                | ---
| スレビュー         | スレビューのレス本文
| メール欄 &#x203B;1 | スレビューやポップアップのレス番号、名前、メール、日付、ID
| ポップアップ       | ポップアップウインドウのレス本文
| アスキーアート     | スレビューやポップアップのアスキーアートと判定されたレス本文
| 板一覧／お気に入り | 板一覧／お気に入りなどサイドバーの項目
| スレ一覧           | スレ一覧の列名と項目
| 書き込みビュー     | 書き込みビューの名前欄、メール欄、レス欄

- &#x203B;1 -- バージョン0.3.0-20200301から追加されました。それより前は「スレビュー」の適用範囲に含まれます。


---

<a name="icons"></a>
### アイコン設定
[キャッシュディレクトリ][cachepriority]内にアイコンテーマのフォルダ(`theme/icons/`)を作成し、
その中に対応するアイコン画像を置くと、JDimのアイコンが置き換わります。
アイコン画像を変更したときは、JDimを再起動してください。

- 画像形式は一般的なものであれば使用可能です。 (png, jpg, svg, gif, bmp, その他)
- 画像ファイルのサイズは任意で構いません。
- ツールバーボタンのアイコンより大きな画像は、 24x24 にサイズ調整されます。[^1] <small>(v0.7.0+から変更)</small>
- ファイル名は「アイコン名.拡張子 」のように指定します。(例: reload.jpg, quit.png)
- ファイル名の一覧は、Gitリポジトリにある [ヘッダファイル(iconfiles.h)][iconfiles] を参照してください。

[^1]: 視認性を良くするため組み込みのアイコン( 16x16 )よりサイズを一回り大きくとります。

---

<a name="example"></a>
### 使用例
[![スキンサンプル][skin_sample_thumb]][skin_sample]
クリックで拡大します。

[キャッシュディレクトリ][cachepriority]内にテーマフォルダ(`theme/`)を作成し、
その中にRes.html(下)と[jd.css][jdcss]をコピーしてJDimを起動します。

```html
<div class="title"><NUMBER/> <NAMELINK/>：<NAME/> <MAIL/></div>
<div class="mes"><MESSAGE/><IMAGE/></div>
<div class="id"><ID/></div>
<div class="date"><DATE/></div>
```

---

<a name="theme"></a>
### テーマ設定

<small>バージョン0.13.0-alpha20241130から追加。</small>

メニューバーの`設定(C)`＞`フォントと色(F)`＞`詳細設定(R)...`を選択すると、フォントと色の詳細設定ダイアログボックスが表示されます。
ダイアログの「テーマの設定」タブからGTKテーマを設定できます。

書き込みビューとツリービューとスレビューの選択範囲には、GTKテーマを上書きする配色設定があります。
上書きを解除してGTKテーマの配色にするには、「色の設定」タブで設定を切り替えます。

> [!NOTE]  
> この機能は実験的なサポートのため、変更または廃止の可能性があります。

#### GTKテーマ (コンボボックス)

GTKテーマを選択してUIテーマ（外観や配色）をまとめて変更できます。
GTKテーマは以下の方法でインストールできます。

- OS・ディストリビューションのパッケージをインストールします。（例：apt, pacman, dnfなど）
- デスクトップ環境のシステム設定からインストールします。（例：KDE System Settingsなど）
- 配布サイトからGTKテーマのアーカイブファイル（zip, tar.gz, tar.xzなど）をダウンロードして、テーマのインストールディレクトリに展開します。
  - 配布サイト: [Gnome-look.org](https://www.gnome-look.org/browse?cat=135&ord=latest "GTK3/4 Themes")
  - インストールディレクトリ: `$HOME/.themes`  (もしディレクトリが存在しない場合は、作成してください。)
  - 例:  [Ant](https://www.gnome-look.org/p/1099856/)のアーカイブファイルをダウンロードして、シェルからインストールします。
    ```sh
    mkdir ~/.themes
    tar xvf Ant.tar.xz
    mv Ant/ ~/.themes
    ```

> [!NOTE]  
> 環境変数 `GTK_THEME` を設定して起動した場合、アプリケーション側のGTKテーマ設定は上書きされるため、テーマの変更はできません。

#### システム設定のGTKテーマを使う (チェックボックス)

チェックすると、デスクトップ環境のシステム設定で選択されているGTKテーマがアプリケーションに適用されます。

> [!TIP]  
> システム設定を適用するにはアプリケーションの再起動が必要です。

#### ダークテーマで表示する (チェックボックス)

チェックすると、アプリケーションをダークテーマ（暗いカラーパターン）に切り替えます。

> [!TIP]  
> 一部のGTKテーマはダークテーマに対応していないため、効果がない場合があります。
> その場合は、ダークテーマに対応したGTKテーマを設定してください。

#### アイコンテーマ (コンボボックス)

<small>バージョン0.13.0-alpha20241214から追加。</small>

アイコンテーマを選択してUIアイコンをまとめて変更できます。
アイコンテーマは以下の方法でインストールできます。

- OS・ディストリビューションのパッケージをインストールします。（例：apt, pacman, dnfなど）
- デスクトップ環境のシステム設定からインストールします。（例：KDE System Settingsなど）
- 配布サイトからGTKテーマのアーカイブファイル（zip, tar.gz, tar.xzなど）をダウンロードして、テーマのインストールディレクトリに展開します。
  - 配布サイト: [Gnome-look.org](https://www.gnome-look.org/browse?cat=132&ord=latest "Full Icon Themes")
  - インストールディレクトリ: `$HOME/.icons`  (もしディレクトリが存在しない場合は、作成してください。)
  - 例:  [Candy icons](https://www.gnome-look.org/p/1305251)のアーカイブファイルをダウンロードして、シェルからインストールします。
    ```sh
    mkdir ~/.icons
    tar xvf candy-icons.tar.xz
    mv candy-icons/ ~/.icons
    ```

> [!TIP]  
> アイコンテーマによっては、ライトテーマかダークテーマのどちらかにしか対応していないため、アイコンが見えない場合があります。

#### システム設定のアイコンテーマを使う (チェックボックス)

チェックすると、デスクトップ環境のシステム設定で選択されているアイコンテーマがアプリケーションに適用されます。

> [!TIP]  
> システム設定を適用するにはアプリケーションの再起動が必要です。

#### シンボリックアイコンで表示する (チェックボックス)

チェックすると、アプリケーションのUIアイコンをシンボリックアイコン（モノクロやシンプルなデザイン）に切り替えます。


[color_value]: https://developer.mozilla.org/ja/docs/Web/CSS/color_value#color_keywords
[iconfiles]: https://github.com/JDimproved/JDim/blob/master/src/icons/iconfiles.h "JDim/iconfiles.h at master"

<!-- 相対URLで GitHub Pages と GitHub.com 両方に対応する -->
[skin_sample_thumb]: ../assets/skin_sample_thumb.png
[skin_sample]: ../assets/skin_sample.png "サンプルのスクリーンショット"
[jdcss]: ../assets/jd.css "サンプルのcss"
[cachepriority]:  ../start/#cachepriority "キャッシュディレクトリの優先順位"
