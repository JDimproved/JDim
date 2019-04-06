<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
    version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:fn="http://www.w3.org/2005/xpath-functions"
    xmlns:regexp="http://exslt.org/regular-expressions"
    exclude-result-prefixes="fn regexp"
>
<xsl:output
    method="html"
    version="4.01"
    encoding="UTF-8"
    doctype-public="-//W3C//DTD HTML 4.01//EN"
    doctype-system="http://www.w3.org/TR/html4/strict.dtd"
    include-content-type="yes"
    indent="yes"
    media-type="text/html"
/>

<!-- 変数 -->
<xsl:variable name="LASTYEAR" select="document( 'history.xml' )//history/year[1]" />
<xsl:variable name="LATEST" select="document( concat( $LASTYEAR, '.xml' ) )//release[last()]" />
<xsl:variable name="VERSION" select="$LATEST/@version" />
<xsl:variable name="RELEASENUM" select="translate( substring( $VERSION, 1, 5 ), '.', '' )" />
<xsl:variable name="VENDOR" select="system-property( 'xsl:vendor' )" />
<xsl:variable name="SAXON" select="$VENDOR='Saxonica' or contains($VENDOR, 'SAXON')" />
<xsl:variable name="PROJECT">JD Project</xsl:variable>
<xsl:variable name="HOMEPAGE">http://jd4linux.sourceforge.jp/</xsl:variable>
<xsl:variable name="ISSUE">&#169;2006-<xsl:value-of select="$LASTYEAR" /></xsl:variable>
<xsl:variable name="COPYRIGHT"><xsl:value-of select="$ISSUE" /><xsl:text>&#32;</xsl:text><xsl:value-of select="$PROJECT" /></xsl:variable>


<!--=================================================================-->
<!--

ノード: /document
変　換: <html>〜</html>
備　考: 全体の基本部分

-->
<xsl:template match="/document">
  <html lang="ja">
    <head>
      <meta name="description" content="JDオンラインマニュアル" />
      <meta name="copyright" content="{ $COPYRIGHT }" />
      <meta name="author" content="jdsksy, tamagodake" />
      <meta name="generator" content="{ $VENDOR }" />
      <title><xsl:value-of select="@header" /></title>
      <link rel="stylesheet" type="text/css" href="main.css" />
      <link rel="index" href="{ $HOMEPAGE }" />
      <link rel="contents" href="{ concat( $HOMEPAGE, 'manual/', $RELEASENUM, '/' ) }" />
      <link rev="made" href="{ $HOMEPAGE }" />
    </head>
    <body>
      <div id="topmenu">
        <a href="./" accesskey="t"><xsl:text>トップページ</xsl:text></a>
        <xsl:text>&#x0A;</xsl:text>
        <a href="{ $HOMEPAGE }" accesskey="h"><xsl:text>ホームページ</xsl:text></a>
        <xsl:text>&#x0A;</xsl:text>
        <a href="http://sourceforge.jp/projects/jd4linux/wiki/FrontPage" accesskey="w"><xsl:text>wiki</xsl:text></a>
        <xsl:text>&#x0A;</xsl:text>
        <span id="version"><xsl:text>Version: </xsl:text><xsl:value-of select="$VERSION" /></span>
      </div>

      <h1><img id="logo" src="image/jd.png" width="96" height="96" alt="JDロゴ" /><xsl:value-of select="@header" /></h1>

      <div id="content">
        <xsl:if test="count( group[@header]|release ) &gt; 1">
          <div id="index">
            <ul class="circle">
              <xsl:apply-templates select="group|release" mode="link" />
            </ul>
          </div>
        </xsl:if>

        <xsl:apply-templates select="*" />
      </div>

      <p id="copyright"><xsl:value-of select="$COPYRIGHT" /></p>
    </body>
  </html>
  <xsl:text>&#x0A;</xsl:text>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

要素: group
変換: <div>〜</div>
用途: グループ分け
備考: ヘッダを付けた物が複数存在するとページ上部に一覧のリンクが表示される

[記載例]

<group header="ヘッダ(任意)">
その他の要素
</group>

-->
<xsl:template match="group" mode="link">
  <xsl:if test="@header">
    <li><a href="#{ translate( generate-id(), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' ) }"><xsl:value-of select="@header" /></a></li>
  </xsl:if>
</xsl:template>

<xsl:template match="group">
  <xsl:if test="@header">
    <p class="marker" id="{ translate( generate-id(), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' ) }"><xsl:value-of select="@header" /></p>
  </xsl:if>

  <div class="group">
    <xsl:if test="@header">
      <h2 class="header"><xsl:value-of select="@header" /></h2>
    </xsl:if>
    <div>
      <xsl:apply-templates select="node()" />
    </div>
  </div>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: issue
変　換: text()
用　途: コピーライト表示に使う発行年の表示

-->
<xsl:template match="issue">
  <xsl:value-of select="$ISSUE" />
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: subgroup
変　換: <div>〜</div>
用　途: サブグループ

[記載例]

<subgroup header="ヘッダ(任意)">
その他の要素
</subgroup>

-->
<xsl:template match="subgroup">
  <xsl:if test="@header">
    <h3 class="header"><xsl:value-of select="@header" /></h3>
  </xsl:if>

  <div class="subgroup"><xsl:apply-templates select="node()" /></div>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: sentence
変　換: <p>〜</p>
用　途: 文の表示

[記載例]

<sentence type="タイプ(任意)">
文
</sentence>

-->
<xsl:template match="sentence">
  <p>
    <xsl:if test="@type">
      <xsl:attribute name="class"><xsl:value-of select="@type" /></xsl:attribute>
    </xsl:if>
    <xsl:apply-templates select="node()" />
  </p>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: list
変　換: <ul><li>〜</li></ul>
用　途: 一覧表示

[記載例]

<list type="タイプ(任意)">
  <item>その他の要素</item>
</list>

-->
<xsl:template match="list">
  <ul>
    <xsl:if test="@type">
      <xsl:attribute name="class"><xsl:value-of select="@type" /></xsl:attribute>
    </xsl:if>
    <xsl:apply-templates select="item" mode="list" />
  </ul>
</xsl:template>

<xsl:template match="item" mode="list">
  <li><div><xsl:apply-templates select="node()" /></div></li>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: descriptions
変　換: <dl><dt>〜</dt><dd>〜</dd></dl>
用　途: 設定項目の解説など

[記載例]

<descriptions caption="キャプション(任意)">
  <item haeder="説明対象">説明内容</item>
  <item icon="アイコンのURI">説明内容</item>
</descriptions>

-->
<xsl:template match="descriptions">
  <xsl:if test="@caption">
    <p class="caption"><xsl:value-of select="@caption" /></p>
  </xsl:if>
  <dl><xsl:apply-templates select="item" mode="dl" /></dl>
</xsl:template>

<xsl:template match="item" mode="dl">
  <xsl:choose>
    <xsl:when test="@icon">
      <dt><img src="{ @icon }" width="16" height="16" alt="アイコン" /></dt>
    </xsl:when>
    <xsl:when test="@header">
      <dt class="oblique"><xsl:value-of select="@header" /></dt>
    </xsl:when>
  </xsl:choose>
  <dd><div><xsl:apply-templates select="node()" /></div></dd>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: asciiart | example | shell
変　換: <pre>〜</pre>
用　途: アスキーアート、記載例、コマンドなどの表示
備　考: 先頭の改行は取り除かれる

[記載例] <pre>なのでインデントに注意

<example caption="キャプション(任意)">
整形済みの文字列
</example>

-->
<xsl:template match="asciiart|example|shell">
  <xsl:if test="@caption">
    <p class="caption"><xsl:value-of select="@caption" /></p>
  </xsl:if>

  <xsl:text>&#x0A;</xsl:text>
  <pre class="{ local-name() }">
    <xsl:choose>
      <xsl:when test="function-available( 'fn:replace' )">
        <xsl:value-of select="fn:replace( text(), '^\n', '', '' )" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of use-when="function-available( 'regexp:replace', 2 )"
                      select="regexp:replace( text(), '^\n', '', '' )" />
      </xsl:otherwise>
    </xsl:choose>
  </pre>
  <xsl:text>&#x0A;</xsl:text>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: image
変　換: <img /> | <a href="URI"><img /></a>
用　途: 画像の表示

[記載例]

<image uri="画像のURI" link="リンクのURI(任意)" width="幅" height="高さ" text="代替テキスト" />

-->
<xsl:template match="image">
  <xsl:choose>
    <xsl:when test="@link">
      <a>
        <xsl:attribute name="href">
          <xsl:apply-templates select="@link" />
        </xsl:attribute>
        <img src="{ @uri }" width="{ @width }" height="{ @height }" alt="{ @text }" />
      </a>
    </xsl:when>
    <xsl:otherwise>
      <img src="{ @uri }" width="{ @width }" height="{ @height }" alt="{ @text }" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: icon
変　換: <img />
用　途: アイコン(16x16)の表示

[記載例]

<icon uri="アイコンのURI" />

-->
<xsl:template match="icon">
  <img src="{ @uri }" width="16" height="16" alt="アイコン" />
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: dialog
変　換: <div><p>ラベル<br /><input type="text" value="〜"></p></div>
用　途: ユーザーコマンドなどのダイアログを表示

[記載例]

<dialog>
  <item label="ラベル" value="内容" />
</dialog>

-->
<xsl:template match="dialog">
  <div class="dialog">
    <xsl:apply-templates select="item" mode="dialog" />
  </div>
</xsl:template>

<xsl:template match="item" mode="dialog">
  <p>
    <xsl:value-of select="@label" /><br />
    <input type="text" value="{ @value }" />
  </p>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: text
変　換: text() | <a href="URI">〜</a>
用　途: 文字を表示| アンカーを設定して文字を表示

[記載例]

<text link="URI">文字</text>

-->
<xsl:template match="text">
  <xsl:choose>
    <xsl:when test="@link">
      <a>
        <xsl:attribute name="href">
          <xsl:apply-templates select="@link" />
        </xsl:attribute>
        <xsl:value-of select="text()" />
      </a>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="text()" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: @link
変　換: SAXONで変換している場合は値を*.xmlから*.htmlに変換する

-->
<xsl:template match="@link">
  <xsl:choose>
    <!-- SAXONで変換している場合は".xml"を".html"に変換する -->
    <xsl:when test="$SAXON=true() and function-available( 'fn:replace' ) and not( starts-with( ., 'http:' ) or starts-with( ., 'https:' ) )">
      <xsl:value-of select="fn:replace( ., '\.xml$', '.html', 'i' )" />
    </xsl:when>
    <!-- Firefox等で直接XMLを表示している場合は変換しない -->
    <xsl:otherwise>
      <xsl:value-of select="." />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: text()
変　換: 連続スペースと改行を取り除く

-->
<xsl:template match="text()">
  <!-- SAXONとFirefoxで"replace()"の実装が異なる -->
  <xsl:choose>
    <xsl:when test="function-available( 'fn:replace' )">
      <xsl:value-of select="fn:replace( ., '^\s+|\n|\s{2,}|\s+$', '', 's' )" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of use-when="function-available( 'regexp:replace', 2 )"
                    select="regexp:replace( ., '^\s+|\n|\s{2,}|\s+$', 'g', '' )" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
<!--=================================================================-->


</xsl:stylesheet>
