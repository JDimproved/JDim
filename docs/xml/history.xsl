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
<xsl:include href="main.xsl" />


<!--=================================================================-->
<!--

ノード: history
変　換: <pre>〜</pre> + <ul><li><a href="YYYY.xml">YYYY</a></li></ul>
用　途: 最新の履歴と年別履歴へのリンクを表示する

[記載例]

<history>
 <year>YYYY</year>
</history>

-->
<xsl:template match="history">
  <div class="release">
    <p class="age">
      <span class="number"><xsl:number format="1" /></span>
      <span class="name_caption"><xsl:text>バージョン</xsl:text></span>
      <xsl:text>：</xsl:text>
      <span class="name">
        <xsl:value-of select="$LATEST/@version" />
        <xsl:if test="not( contains( $LATEST/@version, 'b' ) or contains( $LATEST/@version, 'rc' ) )"><xsl:text> ★</xsl:text></xsl:if>
      </span>
      <span class="mail"><xsl:text> [new]</xsl:text></span>
      <xsl:text>： </xsl:text>
      <xsl:value-of select="$LATEST/@date" />
      <xsl:text> 00:00:00.00</xsl:text>
    </p>

    <xsl:text>&#x0A;</xsl:text>
    <pre class="history">
      <xsl:choose>
        <xsl:when test="function-available( 'fn:replace' )">
          <xsl:value-of select="fn:replace( $LATEST, '^\n', '', '' )" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of use-when="function-available( 'regexp:replace', 2 )"
                        select="regexp:replace( $LATEST, '^\n', '', '' )" />
        </xsl:otherwise>
      </xsl:choose>
    </pre>
    <xsl:text>&#x0A;</xsl:text>

    <ul class="circle">
      <xsl:apply-templates select="year" />
    </ul>
  </div>
</xsl:template>


<xsl:template match="year">
  <li>
    <a>
      <xsl:attribute name="href">
        <xsl:choose>
          <xsl:when test="$SAXON=true()">
            <xsl:value-of select="concat(., '.html')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="concat(., '.xml')" />
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:value-of select="concat( ., '年' )" />
    </a>
  </li>
</xsl:template>
<!--=================================================================-->


<!--=================================================================-->
<!--

ノード: release
変　換: <pre>〜</pre>
用　途: 履歴を表示する

[記載例]

<release version="バージョン" date="日時">
更新内容
</release>

-->
<xsl:template match="release" mode="link">
  <xsl:if test="@version">
    <li><a href="#{ translate( generate-id(), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' ) }"><xsl:value-of select="@version" /></a></li>
  </xsl:if>
</xsl:template>

<xsl:template match="release">
  <p class="marker" id="{ translate( generate-id(), 'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' ) }"><xsl:value-of select="@version" /></p>

  <div class="release">
    <p class="sage">
      <span class="number"><xsl:number format="1" /></span>
      <span class="name_caption"><xsl:text>バージョン</xsl:text></span>
      <xsl:text>：</xsl:text>
      <span class="name">
        <xsl:value-of select="@version" />
        <xsl:if test="not( contains( @version, 'b' ) or contains( @version, 'rc' ) )"><xsl:text> ★</xsl:text></xsl:if>
      </span>
      <xsl:text> [sage]： </xsl:text>
      <xsl:value-of select="@date" />
      <xsl:text> 00:00:00.00</xsl:text>
    </p>

    <xsl:text>&#x0A;</xsl:text>
    <pre class="history">
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
  </div>
</xsl:template>
<!--=================================================================-->


</xsl:stylesheet>
