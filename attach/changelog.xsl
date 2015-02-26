<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
    version="2.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:fn="http://www.w3.org/2005/xpath-functions"
    xmlns:regexp="http://exslt.org/regular-expressions"
>
<xsl:output
    method="text"
    encoding="UTF-8"
    media-type="text/plain"
/>


<xsl:template match="/document">
    <xsl:text>[ 更新履歴 ]</xsl:text>
    <xsl:text>&#x0A;&#x0A;</xsl:text>
    <xsl:text>  以前の更新履歴は&quot;http://jd4linux.sourceforge.jp/manual/&quot;を参照してください。</xsl:text>
    <xsl:text>&#x0A;&#x0A;</xsl:text>
    <xsl:text>  svnのコミットログを知りたい場合は</xsl:text>
    <xsl:text>&#x0A;&#x0A;</xsl:text>
    <xsl:text>   svn log -r 1000:HEAD http://svn.sourceforge.jp/svnroot/jd4linux/jd/trunk</xsl:text>
    <xsl:text>&#x0A;&#x0A;</xsl:text>
    <xsl:text>  などの様にしてsvnサーバから直接取得してください。&#x0A;</xsl:text>
    <xsl:text>  (1000のところは適当なリビジョン番号にしてください)&#x0A;</xsl:text>
    <xsl:text>&#x0A;&#x0A;</xsl:text>

    <xsl:apply-templates select="release[last()]" />
</xsl:template>


<xsl:template match="release">
    <xsl:text>------------------------------------------------------------------------&#x0A;</xsl:text>
    <xsl:value-of select="@version" /> - <xsl:value-of select="@date" /><xsl:text>&#x0A;</xsl:text>
    <xsl:text>------------------------------------------------------------------------&#x0A;</xsl:text>

    <!-- SAXONとFirefox3で"replace()"の実装が異なる -->
    <xsl:choose>
      <xsl:when test="function-available( 'fn:replace' )">
        <xsl:value-of select="fn:replace( text(), '^(.+)$', ' * $1', 'm' )" />
      </xsl:when>
      <xsl:otherwise><!--xsl:when test="function-available( 'regexp:replace' )"-->
        <xsl:value-of use-when="function-available( 'regexp:replace', 2 )"
                      select="regexp:replace( text(), '^(.+)$', 'gm', ' * $1' )" />
      </xsl:otherwise>
    </xsl:choose>
</xsl:template>


</xsl:stylesheet>
