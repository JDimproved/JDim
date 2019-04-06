<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet type="text/xsl" href="main.xsl"?>
<!DOCTYPE document SYSTEM "document.dtd">

<document header="テーマについて">


<group header="スタイルシート設定 (jd.css)">
  <sentence>
  スタイルシートはテーマフォルダ(デフォルトでは~/.jd/theme)を作成し、その中に「jd.css」
  という名前のファイルを作って設定する。
  </sentence>

  <subgroup header="対応しているプロバティ">
    <descriptions>
      <item header="color">文字色: リンクなどJDの設定メニューから設定する物を除く</item>
      <item header="background-color">背景色:</item>
      <item header="border-color">枠線色: border-left-color など個別指定も可能</item>
      <item header="border-style">枠線形: solid のみに対応</item>
      <item header="border-width">枠線幅: border-left-width など個別指定も可能</item>
      <item header="margin">外余白: margin-left など個別指定も可能</item>
      <item header="padding">内余白: padding-left など個別指定も可能 </item>
      <item header="text-align">位置: left, center, right のみに対応</item>
    </descriptions>
  </subgroup>

  <subgroup header="色の指定方法">
    <sentence>
    blackやwhiteなどの定義済み色名か「#RRGGBB」形式で指定する。定義済み
    色名の種類は次の通り。
    </sentence>

    <list type="circle">
      <item>red</item>
      <item>fuchsia</item>
      <item>purple</item>
      <item>maroon</item>
      <item>yellow</item>
      <item>lime</item>
      <item>green</item>
      <item>olive</item>
      <item>blue</item>
      <item>aqua</item>
      <item>teal</item>
      <item>navy</item>
      <item>white</item>
      <item>silver</item>
      <item>gray</item>
      <item>black</item>
      </list>
  </subgroup>

  <subgroup header="単位">
    <sentence>px, em のみに対応。単位を省略すると px になる。</sentence>
  </subgroup>

  <subgroup header="レスの構造">
    <sentence>
    スレ内のひとつひとつのレスは以下のような構造となっている。NUMBERやNA
    MEなどの要素については「レス構造設定」の項で説明する。
    </sentence>

<example>
&lt;div class=&quot;res&quot;&gt;
&lt;div class=&quot;title&quot;&gt;&lt;NUMBER/&gt; &lt;NAMELINK/&gt;：&lt;NAME/&gt; &lt;MAIL/&gt;： &lt;DATE/&gt; &lt;ID/&gt;&lt;/div&gt;
&lt;div class=&quot;mes&quot;&gt;&lt;MESSAGE/&gt;&lt;IMAGE/&gt;&lt;/div&gt;
&lt;/div&gt;
</example>
  </subgroup>

  <subgroup header="定義済みのセレクタ">
    <sentence>
    注意: jd.css でクラス指定する際は div.res{} と書かずに .res{} と要素
    名(div)を書かずにクラス名だけを指定すること。
    </sentence>

    <descriptions>
      <item header="body">スレビューのbody要素</item>
      <item header=".res">ひとつのレスの要素</item>
      <item header=".title">ひとつのレスのヘッダ行</item>
      <item header=".mes">ひとつのレスの本文</item>
      <item header=".separator">ここまで読んだ。&lt;div class=&quot;separator&quot;&gt;ここまで読んだ&lt;/div&gt; という構造になっている。</item>
      <item header=".comment">コメントブロック。&lt;div class=&quot;comment&quot;&gt;任意のコメント&lt;/div&gt; という構造になっている。</item>
      <item header="imgpopup">画像ポップアップのbody要素。color, background-color, border-color, border-width, margin プロバティのみ(単位はpxのみ)に対応。</item>
    </descriptions>
  </subgroup>
</group>


<group header="レス構造設定 (Res.html)">
  <sentence>
  スレ内のひとつひとつのレスは「スタイルシート指定」の「定義済み要素、ク
  ラス」の項で示した構造となっている。この構造をテーマフォルダ(デフ
  ォルトでは~/.jd/theme)の中にRes.htmlという名前のファイルを作って指定すること
  が出来る。
  </sentence>

  <subgroup header="使用可能な要素">
    <sentence>&lt;div&gt; のみ</sentence>
  </subgroup>

  <subgroup header="置換可能な定義済み要素">
    <sentence>
    Res.htmlでは次のように定義済み要素を指定して文字列の置換ができる。
    </sentence>

    <descriptions type="option" summary="置換可能な定義済み要素">
      <item header="&lt;NUMBER/&gt;">レス番号</item>
      <item header="&lt;NAMELINK/&gt;">名前メニュー表示のリンク。「名前」の文字列に置換される。</item>
      <item header="&lt;NAME/&gt;">名前</item>
      <item header="&lt;MAIL/&gt;">メール</item>
      <item header="&lt;DATE/&gt;">日付</item>
      <item header="&lt;ID/&gt;">ID</item>
      <item header="&lt;MESSAGE/&gt;">本文。「br=&quot;no&quot;」属性を指定すると本文の改行をしない。</item>
      <item header="&lt;IMAGE/&gt;">インライン画像</item>
    </descriptions>
  </subgroup>

  <subgroup header="注意点">
    <list type="decimal">
      <item>
      &lt;div class=&quot;res&quot;&gt;〜&lt;/div&gt;はRes.htmlで指定する
      必要が無い。つまり Res.htmlに&quot;hoge&quot;とだけ指定した場合、レ
      スの構造は以下のようになる。
<example>
&lt;div class=&quot;res&quot;&gt;
hoge
&lt;/div&gt;
</example>
      </item>
      <item>div の中に divを配置することは出来ない( 上の &lt;div class=&quot;res&quot;&gt;は例外 )。</item>
      <item>クラス名は定義済みの物だけではなくて自由に指定できる。</item>
    </list>
  </subgroup>
</group>

<group header="アイコン設定">
  <sentence>
   アイコンテーマのフォルダ(デフォルトでは ~/.jd/theme/icons )に
   対応するアイコン画像を置くとJDのアイコンが置き換わる。
   </sentence>

  <subgroup header="">
    <list type="decimal">
   <item>画像形式は一般的な物なら大体使用可能 ( png, jpg, svg, gif, bmp, その他)</item>
   <item>画像サイズは任意のサイズで良い( 16x16 がデフォルトサイズ)</item>
   <item>ファイル名は「アイコン名.拡張子 」(例) reload.jpg、 quit.png</item>
   <item>ファイル名の一覧はsvnにある<text link="http://sourceforge.jp/projects/jd4linux/svn/view/jd/trunk/src/icons/iconfiles.h?view=markup&amp;root=jd4linux">ヘッダファイル(iconfiles.h)</text>を参照すること</item>
   </list>
  </subgroup>
</group>

<group header="使用例">
  <sentence>
    <image link="image/skin_sample.png" uri="image/skin_sample_thumb.png" text="スキンサンプル" />
    クリックで拡大
  </sentence>

  <sentence>Res.html(下)と<text link="jd.css">jd.css</text></sentence>

<example>
&lt;div class=&quot;title&quot;&gt;&lt;NUMBER/&gt; &lt;NAMELINK/&gt;：&lt;NAME/&gt; &lt;MAIL/&gt;&lt;/div&gt;
&lt;div class=&quot;mes&quot;&gt;&lt;MESSAGE/&gt;&lt;IMAGE/&gt;&lt;/div&gt;
&lt;div class=&quot;id&quot;&gt;&lt;ID/&gt;&lt;/div&gt;
&lt;div class=&quot;date&quot;&gt;&lt;DATE/&gt;&lt;/div&gt;
</example>
</group>


</document>
