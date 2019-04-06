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
<xsl:include href="main.xsl" /> 

<xsl:template match="/document">
  <xsl:text>[ </xsl:text><xsl:value-of select="@header" /><xsl:text> ]&#x0A;&#x0A;</xsl:text>
  <xsl:text>ここに書かれていない詳細については&quot;http://jd4linux.sourceforge.jp/&quot;や&#x0A;</xsl:text>
  <xsl:text>&quot;http://sourceforge.jp/projects/jd4linux/wiki/FrontPage&quot;を参照してください。&#x0A;</xsl:text>

  <!-- "about.xml"の内容 -->
  <xsl:apply-templates select="*" />
  <!-- "start.xml"を含める -->
  <xsl:text>&#x0A;</xsl:text>
  <xsl:apply-templates select="document( '../xml/start.xml' )//group" />
</xsl:template>


</xsl:stylesheet>
